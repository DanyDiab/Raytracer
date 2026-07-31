#include <cuda_device_runtime_api.h>
#include <cuda_runtime.h>

#include "headers/Camera.hpp"
#include "headers/GPUMemory.hpp"
#include "headers/HitRecord.hpp"
#include "headers/Hittable.cuh"
#include "headers/PRNG.cuh"
#include "headers/Transform.hpp"
#include <cuda_runtime_api.h>
#include <driver_types.h>
#include <glm/common.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include <chrono>
#include "headers/Ray.cuh"
#include "headers/CameraRayGenerationInfo.hpp"
#include <device_launch_parameters.h>
#include <cuda/std/cmath>
#include "headers/RayAveraging.cuh"

constexpr int maxNumBounces = 10;
// how big is the square for each pixel? square it and this is the number of rays per pixel
constexpr int samples = 1000;

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

__device__ glm::vec3 RayHittableCollision(Raytracer::Ray ray, Raytracer::Hittable* hittables, int numHittables, curandState_t* state, glm::vec3 skyColor, int index){
    // invalid index

    Raytracer::HitRecord hi = ray.RayIntersectShapes(hittables, numHittables);

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

        ray.origin = hitPoint + (hi.normal *.001f);
        ray.dir = ray.determineScatterDirection(hi, state);

        hi = ray.RayIntersectShapes(hittables, numHittables);

        if (hi.hitDistance < 0.0f) {
            incomingLight += skyColor * throughput;
            break;
        }

        outputtedLight += hi.mat.emittedColor;

        incomingLight += outputtedLight * throughput;

        throughput *= (hi.mat.albedo);
        
        numBounced++;
    }

    return incomingLight;
}

__global__ void RenderPass(int numRays, Raytracer::Hittable* hittables, int numHittables, glm::vec3* colors, CameraRayGenerationInfo camInfo, double currTime, curandState_t* prngStates, glm::vec3 skyColor){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);

    if(index < numRays){
        curandState_t prngState = prngStates[index];
        Raytracer::Ray ray = Raytracer::generateRayWithDeviation(camInfo,currTime,index, &prngState);
        glm::vec3 color = RayHittableCollision(ray, hittables, numHittables, &prngState, skyColor, index);
        colors[index] += color;

		prngStates[index] = prngState;
    }

}

GPUMemory initGPUMemory(const std::vector<std::shared_ptr<Raytracer::Hittable>>& hittables, int width, int height){
    int numHittables = hittables.size();
    int numPixels = height * width;
    Raytracer::Hittable *localHittable;
    cudaMalloc(&localHittable, sizeof(Raytracer::Hittable) * numHittables);
    
    for (int i = 0; i < numHittables; i++) {
        Raytracer::Hittable* dest = localHittable + i;
        const Raytracer::Hittable* src = hittables[i].get();
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

	PRNG::initRandStates<<<blocks, threads>>>(currTime, prngStates, numPixels);

    GPUMemory memory;
    memory.colors = colors;
    memory.hittable = localHittable;
    memory.prngStates = prngStates;

    return memory;
}

void launchRenderPass(GPUMemory memory, int numHittables, int numRays, CameraRayGenerationInfo camInfo, double currTime, glm::vec3 skyColor){
    int threads = 256;
    int blocks = (numRays + threads - 1) / threads;

    RenderPass<<<blocks, threads>>>(numRays, memory.hittable, numHittables, memory.colors, camInfo, currTime, memory.prngStates, skyColor);
}



void writeColorsToPPM(std::vector<glm::vec3> colors, int height, int width){
    std::cout << "P3\n" << width << ' ' << height << "\n255\n";

    for(int i = 0; i < colors.size(); i++){
        glm::vec3 color = colors.at(i);

        float ir = color.r * 255.9999f;
        float ig = color.g * 255.9999f; 
        float ib = color.b * 255.9999f;

        std::cout << ir << ' ' << ig << ' ' << ib << '\n';
    }
    std::cout << std::flush;
    // std::cout << std::endl;
}

// 155, 203, 242
void Camera::Render(const std::vector<std::shared_ptr<Raytracer::Hittable>>& hittables){
    // glm::vec3 skyColor = glm::vec3(155 / 255.0,203 / 255.0,242 / 255.0);
    glm::vec3 skyColor = glm::vec3(.2f,.2f,.2f);

    int width = viewportInfo->width;
    int height = viewportInfo->height;
    int numRays = width * height;

    float left = transform.position.x - (width / 2.0f);
    float bot = transform.position.y - (height / 2.0f);

    GPUMemory GPUmemory = initGPUMemory(hittables, width, height);

    CameraRayGenerationInfo camInfo;

    camInfo.botOffset = bot;
    camInfo.leftOffset = left;
    camInfo.camPos = transform.position;
    camInfo.forward = transform.forward();
    camInfo.right = transform.right();
    camInfo.up = transform.up();
    camInfo.width = width;
    camInfo.height = height;
    camInfo.fov = 90.0f;
    camInfo.projectionType = PERSPECTIVE;
    
    for(int i = 0; i < samples; i++){
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        double currTime = std::chrono::duration_cast<std::chrono::nanoseconds>(epoch).count();
        launchRenderPass(GPUmemory, hittables.size(), numRays, camInfo, currTime, skyColor);
    }

    int threads = 256;
    int blocks = (numRays + threads - 1) / threads;

    AverageRayColors<<<blocks, threads>>>(GPUmemory.colors,numRays,samples);


    std::vector<glm::vec3> colors;
    colors.resize(numRays);
    cudaMemcpy(colors.data(), GPUmemory.colors, numRays * sizeof(glm::vec3), cudaMemcpyDeviceToHost);

    
    writeColorsToPPM(colors, height, width);
}
