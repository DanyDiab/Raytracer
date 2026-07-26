#include <cstdio>
#include <glm/ext/quaternion_geometric.hpp>

#include "./headers/Ray.cuh"
#include "./headers/Hittable.cuh"
#include "headers/HitRecord.hpp"
#include "headers/CameraRayGenerationInfo.hpp"
#include "headers/Material.hpp"
#include "headers/PRNG.cuh"


__device__ Raytracer::HitRecord Raytracer::Ray::RayIntersectShapes(Raytracer::Hittable* hittables, const int numHittables){
    Raytracer::HitRecord closestRecord;
    closestRecord.hitDistance = -1.0f;
    closestRecord.normal = glm::vec3(0);
    
    // found closer hit point
    for(int i = 0; i < numHittables; i++){
        Raytracer::Hittable& shape = hittables[i];
        Raytracer::HitRecord rayHR = shape.rayCollide(*this);
        if(rayHR.hitDistance < -.999999f || rayHR.hitDistance < 0.001f) continue;

        // found better hit
        if((closestRecord.hitDistance == -1.0f) || rayHR.hitDistance < closestRecord.hitDistance){

            closestRecord.hitDistance = rayHR.hitDistance;
            closestRecord.normal = rayHR.normal;
            closestRecord.mat = shape.mat;
        }
    }

    return closestRecord;
}

__device__ Raytracer::Ray Raytracer::generateRayWithDeviation(CameraRayGenerationInfo camInfo, double currTime, int index, curandState_t* prngState){

    glm::vec3 deviation = PRNG::randomUnitVec(prngState);

    int pixelX = index % camInfo.width;
    int pixelY = index / camInfo.width;

    glm::vec3 origin = camInfo.camPos + (camInfo.up * (pixelY + camInfo.botOffset + deviation.y)) + (camInfo.right * (pixelX + camInfo.leftOffset +  deviation.x));

    glm::vec3 dir = camInfo.forward;

    Raytracer::Ray ray;

    ray.dir = dir;
    ray.origin = origin;
    
    return ray;
}

__device__ glm::vec3 Raytracer::Ray::determineScatterDirection(Raytracer::HitRecord record, curandState_t* state){
    Material mat = record.mat;

    float metallic = mat.metallic;

    float randT = PRNG::randFloat(state);

    bool metallicScatter = randT < metallic;
    glm::vec3 dir;
    if(metallicScatter){
        // subtract height twice and translate
        dir = this->dir - (2 * glm::dot(this->dir, record.normal) * record.normal);
    }
    else{
        dir = record.normal + PRNG::randomUnitVecSameHemisphere(record.normal, state);
    }

    return glm::normalize(dir);
}
