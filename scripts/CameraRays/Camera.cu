#include <cuda_device_runtime_api.h>
#include <cuda_runtime.h>

#include "../headers/Camera/Camera.hpp"
#include "../headers/Util/GPUMemory.hpp"
#include "../headers/RayHits/HitRecord.hpp"
#include "../headers/Hittables/Hittable.cuh"
#include "../headers/Hittables/BVH.cuh"

#include "../headers/Util/PRNG.cuh"
#include "../headers/Util/Transform.hpp"
#include "../headers/RayHits/Ray.cuh"
#include "../headers/Camera/CameraRayGenerationInfo.hpp"
#include "../headers/Util/RayAveraging.cuh"
#include "../headers/Camera/RenderFlags.hpp"
#include "../headers/Util/GPUTimer.cuh"
#include "../headers/Util/ProgressBar.cuh"


#include <cuda_runtime_api.h>
#include <driver_types.h>
#include <glm/common.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

#include <chrono>
#include <device_launch_parameters.h>
#include <cuda/std/cmath>

constexpr int maxNumBounces = 15;
constexpr int samples = 1;

constexpr int renderTimeSeconds = 60;

Camera::Camera(ViewportInfo vi) {
    transform = {
        .position = glm::vec3(0,0,0),
        .rotation = glm::quat(1,0,0,0),
        .scale = glm::vec3(1,1,1)
    };

    viewportInfo = std::make_unique<ViewportInfo>(vi);
}

Camera::Camera(ViewportInfo vi, glm::vec3 pos, glm::quat rot){
    transform = {
        .position = pos,
        .rotation = rot,
        .scale = glm::vec3(1,1,1)
    };

    viewportInfo = std::make_unique<ViewportInfo>(vi);
}

__device__ glm::vec3 RayHittableCollision(
    Raytracer::Ray ray, 
    Raytracer::Hittable* hittables, 
    int numHittables, 
    curandState_t* state, 
    glm::vec3 skyColor, 
    int index, 
    int renderingFlags,
    BVH::BVHNode* nodes){
    // invalid index

    Raytracer::HitRecord hi = BVH::trace(ray, nodes);

    if(hi.hitDistance < 0.0f){
        return skyColor;
    }

    int numBounced = 1;

    glm::vec3 throughput = hi.mat.albedo;
    glm::vec3 incomingLight = glm::vec3(0);
    glm::vec3 outputtedLight = hi.mat.emittedColor;

    unsigned int seed = (unsigned int)index;
    
    while(numBounced < maxNumBounces){

        glm::vec3 hitPoint = (ray.dir * hi.hitDistance) + ray.origin;


        glm::vec4 scatterDirWithFlag = ray.determineScatterDirection(hi, state);
        glm::vec3 rawScatterDir = scatterDirWithFlag;
        float refractionFlag = scatterDirWithFlag.w;

        ray.dir = rawScatterDir;

        glm::vec3 nudgeDir = refractionFlag > 0 ? ray.dir : hi.normal;
        ray.origin = hitPoint + (nudgeDir *.001f);

        hi = BVH::trace(ray, nodes);

        if (hi.hitDistance < 0.0f) {
            incomingLight += skyColor * throughput;
            break;
        }

        outputtedLight += hi.mat.emittedColor;
        if(renderingFlags & Flags::RenderFlags::DebugNormals){
            incomingLight = hi.normal;
        }
        else{
            incomingLight += outputtedLight * throughput;
        }

        throughput *= (hi.mat.albedo);
        
        numBounced++;
    }

    return incomingLight;
}

__global__ void RenderPass(
    int numRays, 
    Raytracer::Hittable* hittables, 
    int numHittables, 
    glm::vec3* colors, 
    CameraRayGenerationInfo camInfo, 
    double currTime, 
    curandState_t* prngStates, 
    glm::vec3 skyColor, 
    int renderingFlags, 
    BVH::BVHNode* nodes
){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);

    if(index < numRays){
        curandState_t prngState = prngStates[index];
        Raytracer::Ray ray = Raytracer::generateRayWithDeviation(camInfo,currTime,index, &prngState);
        glm::vec3 color = RayHittableCollision(ray, hittables, numHittables, &prngState, skyColor, index, renderingFlags, nodes);
        colors[index] += color;

		prngStates[index] = prngState;
    }

}

GPUMemory initGPUMemory(const std::vector<Raytracer::Hittable> hittables, int width, int height, BVH::BVH bvh){
    int numHittables = hittables.size();
    int numPixels = height * width;
    Raytracer::Hittable *localHittable;
    cudaMalloc(&localHittable, sizeof(Raytracer::Hittable) * numHittables);
    
    for (int i = 0; i < numHittables; i++) {
        Raytracer::Hittable* dest = localHittable + i;
        const Raytracer::Hittable* src = &hittables[i];
        cudaMemcpy(dest, src, sizeof(Raytracer::Hittable), cudaMemcpyHostToDevice);
    }

    glm::vec3* colors;
    int colorBytes = numPixels * sizeof(glm::vec3);

    cudaMalloc(&colors,colorBytes);
    cudaMemset(colors, 0, colorBytes);

    curandState_t* prngStates;

    int stateBytes = sizeof(curandState_t) * numPixels;

    cudaMalloc(&prngStates, stateBytes);
    cudaMemset(prngStates, 0, stateBytes);

	auto now = std::chrono::system_clock::now();
	auto epoch = now.time_since_epoch();
	unsigned long long currTime = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();

	int threads = 256;
	int blocks = (numPixels + threads - 1) / threads;

    int numNodes = bvh.nodes.size();
    BVH::BVHNode* nodes;
    
    cudaMalloc(&nodes, numNodes * sizeof(BVH::BVHNode));

    for (int i = 0; i < numNodes; i++) {
        BVH::BVHNode* dest = nodes + i;
        const BVH::BVHNode* src = &bvh.nodes[i];
        cudaMemcpy(dest, src, sizeof(BVH::BVHNode), cudaMemcpyHostToDevice);
    }


	PRNG::initRandStates<<<blocks, threads>>>(currTime, prngStates, numPixels);

    GPUMemory memory;
    memory.colors = colors;
    memory.hittable = localHittable;
    memory.prngStates = prngStates;
    memory.nodes = nodes;

    return memory;
}

void launchRenderPass(GPUMemory memory, int numHittables, int numRays, CameraRayGenerationInfo camInfo, double currTime, glm::vec3 skyColor, int renderingFlags, cudaStream_t stream){
    int threads = 256;
    int blocks = (numRays + threads - 1) / threads;

    RenderPass<<<blocks, threads, 0, stream>>>(
        numRays, 
        memory.hittable,
        numHittables, 
        memory.colors, 
        camInfo, 
        currTime, 
        memory.prngStates, 
        skyColor, 
        renderingFlags, 
        memory.nodes
    );
}





std::vector<glm::vec3> Camera::Render(const std::vector<Raytracer::Hittable> hittables, int flags){
    bool timerEnabled = flags & Flags::RenderFlags::Performance;

    cudaStream_t stream;

    cudaStreamCreate(&stream);
    Time::GPUTimer timer(timerEnabled, stream);
    glm::vec3 skyColor = glm::vec3(155 / 255.0,203 / 255.0,242 / 255.0);
    // glm::vec3 skyColor = glm::vec3(0);

    int width = viewportInfo->width;
    int height = viewportInfo->height;
    int numRays = width * height;

    float left = transform.position.x - (width / 2.0f);
    float bot = transform.position.y - (height / 2.0f);
    BVH::BVH bvh = BVH::BVH(hittables);

    timer.addMarker("INIT GPU MEMORY");
    GPUMemory GPUmemory = initGPUMemory(hittables, width, height, bvh);
    timer.addMarker("INIT GPU MEMORY");
    CameraRayGenerationInfo camInfo;

    camInfo.botOffset = bot;
    camInfo.leftOffset = left;
    camInfo.camPos = transform.position;
    camInfo.forward = transform.forward();
    camInfo.right = transform.right();
    camInfo.up = transform.up();
    camInfo.width = width;
    camInfo.height = height;
    camInfo.fov = 60.0f;
    camInfo.projectionType = PERSPECTIVE;
    
    std::vector<float> progressData(samples);


    timer.addMarker("Render Pass");
    for(int i = 0; i < samples; i++){
        progressData[i] = static_cast<float>(i + 1) / static_cast<float>(samples);

        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        double currTime = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();

        launchRenderPass(GPUmemory, hittables.size(), numRays, camInfo, currTime, skyColor, flags, stream);

        if(flags & Flags::Progress){
            cudaLaunchHostFunc(stream, ProgressCallBack, &progressData[i]);
        }
        
        timer.addMarker("Render Pass");
    }
    


    int threads = 256;
    int blocks = (numRays + threads - 1) / threads;

    timer.addMarker("Ray Averaging");
    AverageRayColors<<<blocks, threads, 0, stream>>>(GPUmemory.colors,numRays,samples);
    timer.addMarker("Ray Averaging");



    std::vector<glm::vec3> colors;
    colors.resize(numRays);
    timer.addMarker("GPU TO CPU MEMCPY");
    cudaMemcpy(colors.data(), GPUmemory.colors, numRays * sizeof(glm::vec3), cudaMemcpyDeviceToHost);
    timer.addMarker("GPU TO CPU MEMCPY");

    std::cout << std::endl;

    if(timerEnabled){
        timer.printAllGroupTimes();
    }

    cudaStreamDestroy(stream);

    return colors;
}
