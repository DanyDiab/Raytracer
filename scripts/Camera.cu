#include <cuda_runtime.h>

#include "headers/Camera.hpp"
#include "headers/HitRecord.hpp"
#include "headers/Hittable.cuh"
#include "headers/Transform.hpp"
#include <cmath>
#include <cuda_runtime_api.h>
#include <glm/common.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

#include "headers/Ray.hpp"
#include <device_launch_parameters.h>
#include <cuda/std/cmath>
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


__global__ void sendRays(Raytracer::Ray* rays, glm::vec3 forward, glm::vec3 right, glm::vec3 up, int width, int height, glm::vec3 camPos, float leftOffset, float botOffset){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);
    if(index < width * height){
        int x = index % width;
        int y = index / width;

        float localX = leftOffset + (float)x;
        float localY = botOffset + (float)y;

        glm::vec3 origin = camPos + (up * localY) + right * (localX);

        rays[index].dir = forward;
        rays[index].origin = origin;

    }
}

// send viewport height * width rays into the scene, from the camera to each pixel
// orthographic projection
void Camera::generateRays(){
    float width = viewportInfo->width;
    float height = viewportInfo->height;

    float left = transform.position.x - (width / 2);
    float bot = transform.position.y - (height / 2);

    int size = width * height;

    rays = std::vector<Raytracer::Ray>(size);

    Raytracer::Ray* rawRay = nullptr;

    cudaMallocManaged(&rawRay, size * sizeof(Raytracer::Ray));

    int threads = 256;
    int blocks = (size + threads - 1) / threads;
    sendRays<<<blocks, threads>>>(rawRay,transform.forward(),transform.right(), transform.up(),width,height,transform.position,left,bot);

    cudaDeviceSynchronize();

    rays.resize(size);
    std::copy(rawRay, rawRay + size, rays.begin());

    cudaFree(rawRay);
}

__global__ void RayHittableCollision(Raytracer::Ray* rays, int numRays, Raytracer::Hittable* hittables, int numHittables, Raytracer::HitRecord* records){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);

    // invalid index

    if(index >= numRays * numHittables){
        return;
    }

    int rayIndex = index % numRays;
    int shapeIndex = index % numHittables;

    Raytracer::Ray& ray = rays[rayIndex];
    Raytracer::Hittable& shape = hittables[shapeIndex];
    Raytracer::HitRecord& record = records[rayIndex];

    float rayHitDistance = shape.rayCollide(ray);

    // found closer hit point
    if(rayHitDistance != -1 && rayHitDistance < record.hitDistance){
        record.ray = ray;
        record.hitDistance = rayHitDistance;
        record.shape = shape;
    }
}

void Camera::launchCollisionKernel(const std::vector<std::shared_ptr<Raytracer::Hittable>>& hittables){
    Raytracer::Ray* raysLocal = nullptr;
    int numRays = rays.size();

    int rayBytes = numRays * sizeof(Raytracer::Ray);

    cudaMallocManaged(&raysLocal, rayBytes);
    memcpy(raysLocal, rays.data(), rayBytes);

    Raytracer::Hittable* hittableLocal; 
    int numHittables = hittables.size();
    int hittableBytes = numHittables * sizeof(Raytracer::Hittable);

    cudaMallocManaged(&hittableLocal, hittableBytes);

    for (int i = 0; i < numHittables; i++) {
        if (!hittables[i]) {
            memset(&hittableLocal[i], 0, sizeof(Raytracer::Hittable));
            continue;
        }
        memcpy(&hittableLocal[i], hittables[i].get(), sizeof(Raytracer::Hittable));
    }

    Raytracer::HitRecord* localRecords;

    cudaMallocManaged(&localRecords,numRays * sizeof(Raytracer::HitRecord));

    for (int i = 0; i < numRays; i++) {
        localRecords[i].hitDistance = -1; 
    }

    int threads = 256;
    int blocks = (numHittables * numRays + threads - 1) / threads;
    RayHittableCollision<<<blocks, threads>>>(raysLocal, numRays, hittableLocal, numHittables, localRecords);
    

    cudaError_t launchErr = cudaGetLastError();
    if (launchErr != cudaSuccess) {
        std::cerr << "Kernel launch failed: " << cudaGetErrorString(launchErr) << std::endl;
    }
    cudaDeviceSynchronize();

    cudaError_t syncErr = cudaDeviceSynchronize();
    if (syncErr != cudaSuccess) {
        std::cerr << "CUDA Sync Error: " << cudaGetErrorString(syncErr) << std::endl;
        exit(EXIT_FAILURE); 
    }

    hitRecords.resize(numRays);

    for(int i = 0; i < numRays; i++){
        auto record = localRecords[i];

        if(record.hitDistance != -1){
            std::cout << "found a hit \n" << record.hitDistance; 
        }
    }
    std::copy(localRecords, localRecords + numRays, hitRecords.begin());

    
    cudaFree(localRecords);
    cudaFree(hittableLocal);
    cudaFree(raysLocal);
}




void writeColorsToPPM(std::vector<glm::vec3> colors, float height, float width){
    std::cout << "P3\n" << width << ' ' << height << "\n255\n";

    for(int i = 0; i < colors.size(); i++){
        glm::vec3 color = colors.at(i);

        float ir = color.r * 255.9999f;
        float ig = color.g * 255.9999f; 
        float ib = color.b * 255.9999f;

        std::cout << ir << ' ' << ig << ' ' << ib << '\n';
    }
    std::cout << std::flush;
}

void Camera::shootRays(const std::vector<std::shared_ptr<Raytracer::Hittable>>& hittables){
    glm::vec3 backgroundColor = glm::vec3(0,0,0);

    float width = viewportInfo->width;
    float height = viewportInfo->height;

    float bot = transform.position.y - (height / 2);

    float size = width * height;
    std::vector<glm::vec3> colors;
    colors.reserve(size);
    
    launchCollisionKernel(hittables);

// TODO
// USE RECORDS!
    for(const auto& hit : hitRecords){
        if(hit.hitDistance != -1){
            colors.push_back(hit.shape.mat.color);
        }
        else{
            colors.push_back(backgroundColor);
        }
    }
    // writeColorsToPPM(colors, height, width);
}


