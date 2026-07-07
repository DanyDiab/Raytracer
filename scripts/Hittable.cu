#include <cuda_runtime.h>
#include <cstdio>

#include "headers/HitRecord.hpp"
#include "headers/Hittable.cuh"
#include "headers/Ray.cuh"
#include "headers/Sphere.cuh"

namespace Raytracer {
    __host__ __device__ Hittable::Hittable(Sphere sphere){
        shapeType = SHAPE_SPHERE;
        this->sphere = sphere;
    }

    __device__ Raytracer::HitRecord Hittable::rayCollide(const Raytracer::Ray ray) const{
        // printf("Inside Hittable: %f", sphere.radius);
        Raytracer::HitRecord hi = {
            .hitDistance = -1.0f,
            .normal = glm::vec3(0),
        };

        if(shapeType == SHAPE_SPHERE){
            float distance = SphereRayCollide(sphere, ray);
            if(distance == -1.0f){
                return hi;
            }
            glm::vec3 normal = SphereRayNormal(sphere, ray, distance);
            hi.hitDistance = distance;
            hi.normal = normal;
        }
        else{
        }

        return hi;
    }



}