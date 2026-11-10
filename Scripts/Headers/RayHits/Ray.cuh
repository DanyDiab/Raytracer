#pragma once

#include "HitRecord.hpp"
#include <curand_kernel.h>
#include <glm/geometric.hpp>
#include <glm/vec4.hpp>
#include "../Camera/CameraRayGenerationInfo.hpp"


namespace Raytracer{
    struct Ray{
        glm::vec3 origin;
        // normalized
        glm::vec3 dir;
        
        Ray() = default;
        inline Ray(glm::vec3 origin, glm::vec3 dir){
            this->origin = origin;
            float mag = glm::length(dir);
            // normalize
            
            if(mag <= 0.0f){
                this->dir = glm::vec3(0.0f, 0.0f, -1.0f);
                return;
            }
            this->dir = dir / mag;
        }

        __device__ glm::vec4 determineScatterDirection(Raytracer::HitRecord record, curandState_t* state);

        __device__ Raytracer::HitRecord RayIntersectShapes(Raytracer::Hittable* hittables, const int numHittables);
    };
    __device__ Raytracer::Ray generateRayWithDeviation(CameraRayGenerationInfo camInfo, double currTime, int index, curandState_t* prngState);
    
  



}
