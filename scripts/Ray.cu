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

    int rasterX = index % camInfo.width;
    int rasterY = index / camInfo.width;

    Raytracer::Ray ray;

    switch(camInfo.projectionType){
        case ORTHOGRAPHIC:{
            glm::vec3 pixelPos = camInfo.camPos + (camInfo.up * (rasterY + camInfo.botOffset + deviation.y)) + (camInfo.right * (rasterX + camInfo.leftOffset +  deviation.x));

            glm::vec3 origin = pixelPos;

            glm::vec3 dir = camInfo.forward;

            ray.dir = dir;
            ray.origin = origin;
            break;
        }
        case PERSPECTIVE:{
            // add .5 to be in the middle of the pixel
            float NDCx = (static_cast<float>(rasterX) + .5f) / camInfo.width;
            float NDCy = (static_cast<float>(rasterY) + .5f) / camInfo.height;
            
            float screenSpaceX = (NDCx * 2) - 1;
            // flip Y to match screen space
            float screenSpaceY = 1 - (NDCy * 2);
            
            float aspectRatio = static_cast<float>(camInfo.width) / camInfo.height;
            
            float fovRadians = glm::radians(camInfo.fov);
            float fovScaler = tan(fovRadians * .5f);


            float pixelX = screenSpaceX * aspectRatio * fovScaler;
            float pixelY = screenSpaceY * fovScaler;

            glm::vec3 camPlanePos = glm::normalize(glm::vec3(camInfo.forward + (camInfo.right * pixelX) + (camInfo.up * pixelY)));

            glm::vec3 origin = camInfo.camPos;
            glm::vec3 dir = camPlanePos;

            ray.dir = dir;
            ray.origin = origin;
            break;
        }
        default:
            printf("unkown cam type");
            break;

    }

    return ray;
}

__device__ glm::vec3 Raytracer::Ray::determineScatterDirection(Raytracer::HitRecord record, curandState_t* state){
    Material mat = record.mat;

    float metallic = mat.metallic;

    float randT = PRNG::randFloat(state);

    bool metallicScatter = randT < metallic;
    glm::vec3 dir;
    if(metallicScatter){
        float roughness = mat.roughness;
        // subtract height twice and translate

        glm::vec3 reflect = this->dir - (2 * glm::dot(this->dir, record.normal) * record.normal);
        // add rand scaled by roughness
        glm::vec3 fuzz = PRNG::randomUnitVec(state) * roughness;
        dir = reflect + fuzz;
    }
    else{
        dir = record.normal + PRNG::randomUnitVecSameHemisphere(record.normal, state);
    }

    return glm::normalize(dir);
}
