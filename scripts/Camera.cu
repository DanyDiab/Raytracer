#include <cstddef>
#include <cuda_runtime.h>

#include "headers/Camera.hpp"
#include "headers/HitRecord.hpp"
#include "headers/Hittable.cuh"
#include "headers/Math.cuh"
#include "headers/RayAveraging.cuh"
#include "headers/Transform.hpp"
#include <cmath>
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

#include "headers/Ray.cuh"
#include <device_launch_parameters.h>
#include <cuda/std/cmath>

constexpr int maxNumBounces = 10;
// how big is the square for each pixel? square it and this is the number of rays per pixel
constexpr int squarePixelSize = 5;

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




__global__ void sendRays(Raytracer::Ray* rays, glm::vec3 forward, glm::vec3 right, glm::vec3 up, glm::vec3 camPos, float leftOffset, float botOffset, int width, int height, int squarePixelSize){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);
    int raysPerPixel = squarePixelSize * squarePixelSize;
    if(index < width * height * raysPerPixel){

        int pixelIndex = index / raysPerPixel;

        int pixelX = pixelIndex % width;
        int pixelY = pixelIndex / width;

        int subPixelIndex = index % raysPerPixel;

        int col = subPixelIndex % squarePixelSize;
        int row = subPixelIndex / squarePixelSize;
        
        float delta = 1.0f / (float) squarePixelSize;

        float x = delta * col;
        float y = delta * row;

        float localX = pixelX + leftOffset + x;
        float localY = pixelY + botOffset + y;

        glm::vec3 origin = camPos + (up * localY) + right * (localX);
        rays[index].dir = forward;
        rays[index].origin = origin;
    }
}

// send viewport height * width rays into the scene, from the camera to each pixel
// orthographic projection
void Camera::generateRays(){
    int width = viewportInfo->width;
    int height = viewportInfo->height;

    float left = transform.position.x - (width / 2.0f);
    float bot = transform.position.y - (height / 2.0f);

    int raysPerPixel = squarePixelSize * squarePixelSize;

    int numPixels =  width * height;
    int size = numPixels * raysPerPixel;

    rays = std::vector<Raytracer::Ray>(size);

    Raytracer::Ray* rawRay = nullptr;

    cudaMallocManaged(&rawRay, size * sizeof(Raytracer::Ray));

    int threads = 256;
    int blocks = (size + threads - 1) / threads;
    sendRays<<<blocks, threads>>>(rawRay,transform.forward(),transform.right(), transform.up(),transform.position, left,bot, width,height, squarePixelSize);

    cudaDeviceSynchronize();

    rays.resize(size);
    cudaError_t copyErr = cudaMemcpy(rays.data(), rawRay, size, cudaMemcpyDeviceToHost);
    if (copyErr != cudaSuccess) {
        std::cout << "something wnet wrong while copying ray data over COPYING " << size * sizeof(Raytracer::Ray) << " Bytes";
        cudaFree(rawRay);
        return;
    }

    cudaFree(rawRay);
}

__global__ void RayHittableCollision(Raytracer::Ray* rays, int numRays, Raytracer::Hittable* hittables, int numHittables, Raytracer::HitRecord* records){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);

    // invalid index

    if(index >= numRays){
        return;
    }

    Raytracer::Ray ray = rays[index];

    Raytracer::HitRecord hi = ray.RayIntersectShapes(hittables, numHittables);

    if(hi.hitDistance < 0.0f){
        records[index].hitDistance = -1.0f;
        records[index].color = glm::vec3(0.0f);
        return;
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

        accumulatedColor *= (hi.color * .5f);

        numBounced++;
    }

    finalRecord.color = accumulatedColor;
    records[index] = finalRecord;
}

void Camera::launchCollisionKernel(const std::vector<std::shared_ptr<Raytracer::Hittable>>& hittables){
    Raytracer::Ray* raysLocal = nullptr;
    int numRays = rays.size();

    int rayBytes = numRays * sizeof(Raytracer::Ray);

    cudaMallocManaged(&raysLocal, rayBytes);
    // 
    cudaMemcpy(raysLocal, rays.data(), rayBytes, cudaMemcpyHostToDevice);


    Raytracer::Hittable* hittableLocal; 
    int numHittables = hittables.size();
    int hittableBytes = numHittables * sizeof(Raytracer::Hittable);

    cudaMallocManaged(&hittableLocal, hittableBytes);

    for (int i = 0; i < numHittables; i++) {
        if (!hittables[i]) {
            std::cout << "this shape isnt initlized index " << i;
            cudaMemset(&hittableLocal[i], 0, sizeof(Raytracer::Hittable));
            continue;
        }
        cudaMemcpy(&hittableLocal[i], hittables[i].get(), sizeof(Raytracer::Hittable), cudaMemcpyHostToDevice);
    }

    Raytracer::HitRecord* localRecords;

    cudaMallocManaged(&localRecords,numRays * sizeof(Raytracer::HitRecord));

    int threads = 256;
    int blocks = (numRays + threads - 1) / threads;
    RayHittableCollision<<<blocks, threads>>>(raysLocal, numRays, hittableLocal, numHittables, localRecords);
    

    cudaError_t launchErr = cudaGetLastError();
    if (launchErr != cudaSuccess) {
        std::cerr << "Kernel launch failed: " << cudaGetErrorString(launchErr) << std::endl;
    }

    cudaError_t syncErr = cudaDeviceSynchronize();
    if (syncErr != cudaSuccess) {
        std::cerr << "CUDA Sync Error: " << cudaGetErrorString(syncErr) << std::endl;
        exit(EXIT_FAILURE); 
    }

    hitRecords.resize(numRays);

    cudaMemcpy(hitRecords.data(), localRecords, numRays * sizeof(Raytracer::HitRecord), cudaMemcpyDeviceToHost);

    cudaFree(localRecords);
    cudaFree(hittableLocal);
    cudaFree(raysLocal);
}

void writeColorsToPPM(std::vector<ColorAverage> colors, int height, int width){
    std::cout << "P3\n" << width << ' ' << height << "\n255\n";

    for(int i = 0; i < colors.size(); i++){
        glm::vec3 color = colors.at(i).color;

        float ir = color.r * 255.9999f;
        float ig = color.g * 255.9999f; 
        float ib = color.b * 255.9999f;

        std::cout << ir << ' ' << ig << ' ' << ib << '\n';
    }
    // std::cout << std::flush;
    std::cout << std::endl;
}



void Camera::shootRays(const std::vector<std::shared_ptr<Raytracer::Hittable>>& hittables){
    glm::vec3 backgroundColor = glm::vec3(0,0,0);

    int width = viewportInfo->width;
    int height = viewportInfo->height;

    float bot = transform.position.y - (height / 2.0f);

    float size = width * height;

    
    launchCollisionKernel(hittables);

    int pixelIndex = 0;
    int raysPerPixel = squarePixelSize * squarePixelSize;
   
    ColorAverage* ca;
    
    Raytracer::HitRecord* localRecords;

    int numRecords = hitRecords.size();

    cudaMallocManaged(&localRecords, sizeof(Raytracer::HitRecord) * numRecords);

    cudaMemcpy(hitRecords.data(), localRecords, numRecords, cudaMemcpyHostToDevice);

    cudaMallocManaged(&ca, sizeof(ColorAverage) * size);

    rayAverage(ca, localRecords, numRecords, size, raysPerPixel);

    std::vector<ColorAverage> colors;
    colors.reserve(size);

    cudaMemcpy(colors.data(), ca, size, cudaMemcpyDeviceToHost);

    cudaFree(localRecords);
    cudaFree(ca);

    writeColorsToPPM(colors, height, width);

}
