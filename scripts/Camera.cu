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
#include <cuda_runtime.h>
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

    rays = std::vector<Raytracer::Ray*>(size);

    Raytracer::Ray** rawRay = nullptr;
    cudaMallocManaged(&rawRay, size * sizeof(Raytracer::Ray));

    int threads = 256;
    int blocks = (size + threads - 1) / threads;
    sendRays<<<blocks, threads>>>(*rawRay,transform.forward(),transform.right(), transform.up(),width,height,transform.position,left,bot);

    cudaDeviceSynchronize();

    rays.resize(size);
    std::copy(rawRay, rawRay + size, rays.begin());

    cudaFree(rawRay);
}

__global__ void RayHittableCollision(Raytracer::Ray** rays, int numRays, Raytracer::Hittable** hittables, int numHittables, Raytracer::HitRecord** records){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);

    // invalid index
    if(index >= numRays * numHittables){
        return;
    }

    int rayIndex = index / numRays;
    int shapeIndex = index / numHittables;

    Raytracer::Ray* ray = rays[rayIndex];
    Raytracer::Hittable* shape = hittables[shapeIndex];
    Raytracer::HitRecord* record = records[shapeIndex];

    float rayHitDistance = shape->rayCollide(ray);

    // found closer hit point
    if(rayHitDistance != -1 && rayHitDistance < record->hitDistance){
        record->ray = ray;
        record->hitDistance = rayHitDistance;
    }
}

std::vector<Raytracer::HitRecord> Camera::launchCollisionKernel(const std::vector<std::shared_ptr<Raytracer::Hittable*>>& hittables){

    Raytracer::Ray** raysLocal = nullptr;
    int numRays = rays.size();

    int rayBytes = numRays * sizeof(Raytracer::Ray*);

    cudaMallocManaged(raysLocal, rayBytes);
    raysLocal = rays.data();

    Raytracer::Hittable** hittableLocal; 
    int numHittables = hittables.size();

    cudaMallocManaged(raysLocal, rayBytes);
    hittableLocal = hittables.data()->get();

    Raytracer::HitRecord* localRecords = nullptr;

    cudaMallocManaged(&localRecords, numHittables * sizeof(Raytracer::HitRecord));


    int threads = 256;
    int blocks = (numHittables * numRays + threads - 1) / threads;
    RayHittableCollision<<<threads, blocks >>>(raysLocal, numRays, hittableLocal, numHittables, &localRecords);

    cudaDeviceSynchronize();

    std::vector<Raytracer::HitRecord> records;

    std::copy(localRecords, localRecords + numHittables, records.begin());

    cudaFree(localRecords);
    cudaFree(hittableLocal);
    cudaFree(raysLocal);

    return records;


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

void Camera::shootRays(const std::vector<std::shared_ptr<Raytracer::Hittable*>>& hittables){
    glm::vec3 backgroundColor = glm::vec3(0,0,0);

    float width = viewportInfo->width;
    float height = viewportInfo->height;

    float bot = transform.position.y - (height / 2);

    float size = width * height;
    std::vector<glm::vec3> colors;
    colors.reserve(size);

    
    launchCollisionKernel(hittables);
    // for(int i = 0; i < size; i++){
    //     Raytracer::Ray* ray = rays.at(i);

    //     float closestHit = std::numeric_limits<float>::max();
    //     std::shared_ptr<Raytracer::Hittable> closestShape;
    //     bool foundHit = false;
    //     for(const auto& shape: hittables){
    //         float hitDistance = shape->rayCollide(ray);
    //         // if missed
    //         if(hitDistance < 0){
    //             continue;
    //         }
    //         // hit
    //         else if(hitDistance < closestHit){
    //             foundHit = true;
    //             closestShape = shape;
    //             closestHit = hitDistance;
    //         }
    //     }
    //     if(foundHit){
    //         glm::vec3 hitPoint = ray->origin + (ray->dir * closestHit);
    //         if(closestShape->shapeType == Raytracer::SHAPE_SPHERE){
    //             glm::vec3 normal = glm::normalize(hitPoint - closestShape->Geometry.sphere.position);
    //             colors.push_back(closestShape->mat.color);
    //         }
    //     }
    //     else{
    //         colors.push_back(backgroundColor);
    //     }

    // }
    writeColorsToPPM(colors, height, width);
}


