#include <cuda_runtime.h>

#include "headers/Camera.hpp"
#include "headers/HitRecord.hpp"
#include "headers/Hittable.cuh"
#include "headers/Math.cuh"
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
#include <tuple>
#include <vector>

#include <chrono>
#include "headers/Ray.cuh"
#include "headers/CameraRayGenerationInfo.hpp"
#include <device_launch_parameters.h>
#include <cuda/std/cmath>

constexpr int maxNumBounces = 10;
// how big is the square for each pixel? square it and this is the number of rays per pixel
constexpr int squarePixelSize = 5;

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




// send viewport height * width rays into the scene, from the camera to each pixel
// orthographic projection

 



__device__ glm::vec3 RayHittableCollision(Raytracer::Ray ray, Raytracer::Hittable* hittables, int numHittables){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);

    // invalid index

    Raytracer::HitRecord hi = ray.RayIntersectShapes(hittables, numHittables);

    if(hi.hitDistance < 0.0f){
        return glm::vec3(0.0f);
    }

    Raytracer::HitRecord finalRecord = hi;

    int numBounced = 0;

    glm::vec3 accumulatedColor = hi.color;
    unsigned int seed = (unsigned int)index;
    while(numBounced < maxNumBounces){

        glm::vec3 hitPoint = (ray.dir * hi.hitDistance) + ray.origin;

        ray.origin = hitPoint + (hi.normal *.001f);
        ray.dir = hi.normal + Raytracer::randomUnitVecSameHemisphere(hi.normal, seed);

        hi = ray.RayIntersectShapes(hittables, numHittables);

        if (hi.hitDistance < 0.0f) {
            accumulatedColor *= glm::vec3(1.0f, 1.0f, 1.0f);
            break;
        }

        accumulatedColor *= (hi.color);

        numBounced++;
    }

    return accumulatedColor;
}

__global__ void RenderPass(int numRays, Raytracer::Hittable* hittables, int numHittables, glm::vec3* colors, CameraRayGenerationInfo camInfo, double currTime){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);

    if(index < numRays){
        Raytracer::Ray ray = Raytracer::generateRayWithDeviation(camInfo,currTime,index);
        glm::vec3 color = RayHittableCollision(ray, hittables, numHittables);
        colors[index] += color;
    }

}

std::tuple<Raytracer::Hittable*, glm::vec3*> initGPUMemory(const std::vector<std::shared_ptr<Raytracer::Hittable>>& hittables, int width, int height){
    int numHittables = hittables.size();
    int numPixels = height * width;
    Raytracer::Hittable *localHittable;
    cudaMalloc(&localHittable, sizeof(Raytracer::Hittable) * numHittables);
    
    for (int i = 0; i < numHittables; i++) {
        Raytracer::Hittable* dest = localHittable + i;
        const Raytracer::Hittable* src = hittables[i].get();
        cudaMemcpy(dest, src, sizeof(Raytracer::Hittable), cudaMemcpyHostToDevice);
    }
    cudaMemcpy(localHittable, &hittables, sizeof(Raytracer::Hittable) * numHittables, cudaMemcpyHostToDevice);
    glm::vec3* colors;
    int colorBytes = numPixels * sizeof(glm::vec3);

    cudaMalloc(&colors,colorBytes);
    cudaMemset(colors, 0, colorBytes);
    return std::make_tuple(localHittable,colors);
}

void launchRenderPass(Raytracer::Hittable* hittables, int numHittables, glm::vec3* colors, int numRays, CameraRayGenerationInfo camInfo, double currTime){
    int threads = 256;
    int blocks = (numRays + threads - 1) / threads;

    RenderPass<<<blocks, threads>>>(numRays, hittables, numHittables, colors, camInfo, currTime);
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


void Camera::Render(const std::vector<std::shared_ptr<Raytracer::Hittable>>& hittables){
    glm::vec3 backgroundColor = glm::vec3(0,0,0);

    int width = viewportInfo->width;
    int height = viewportInfo->height;
    int numRays = width * height;

    float left = transform.position.x - (width / 2.0f);
    float bot = transform.position.y - (height / 2.0f);

    std::tuple<Raytracer::Hittable*, glm::vec3*> memoryTuple = initGPUMemory(hittables, width, height);

    Raytracer::Hittable* hittablesPTR = std::get<0>(memoryTuple);
    glm::vec3* colorsPTR = std::get<1>(memoryTuple);

    CameraRayGenerationInfo camInfo;

    camInfo.botOffset = bot;
    camInfo.leftOffset = left;
    camInfo.camPos = transform.position;
    camInfo.forward = transform.forward();
    camInfo.right = transform.right();
    camInfo.up = transform.up();
    camInfo.width = width;
    camInfo.height = height;
    

    for(int i = 0; i < 100; i++){
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        double currTime = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
        launchRenderPass(hittablesPTR, hittables.size(), colorsPTR, numRays, camInfo, currTime);
    }

    std::vector<glm::vec3> colors;

    colors.resize(numRays);
    cudaMemcpy(colors.data(), colorsPTR, numRays * sizeof(glm::vec3), cudaMemcpyDeviceToHost);

    
    writeColorsToPPM(colors, height, width);
}
