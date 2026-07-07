#include <cstdio>

#include "./headers/Ray.cuh"
#include "./headers/Hittable.cuh"
#include "headers/HitRecord.hpp"

__device__ Raytracer::HitRecord Raytracer::Ray::RayIntersectShapes(Raytracer::Hittable* hittables, const int numHittables){
    Raytracer::HitRecord closestRecord;
    closestRecord.hitDistance = -1.0f;
    closestRecord.color = glm::vec3(0);
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
            glm::vec3 newColor = shape.mat.color;

            closestRecord.color = newColor;
        }
    }

    return closestRecord;
}