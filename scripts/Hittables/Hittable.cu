#include <cuda_runtime.h>
#include <cstdio>

#include "../headers/RayHits/HitRecord.hpp"
#include "../headers/Hittables/Hittable.cuh"
#include "../headers/RayHits/Ray.cuh"
#include "../headers/Hittables/Sphere.cuh"

namespace Raytracer {
    __host__ __device__ Hittable::Hittable(Sphere sphere){
        shapeType = SHAPE_SPHERE;
        this->sphere = sphere;
    }

    // __host__ __device__ Hittable::Hittable()

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