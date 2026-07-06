#include <cuda_runtime.h>


#include "headers/Hittable.cuh"
#include "headers/Sphere.cuh"

namespace Raytracer {


    __host__ __device__ Hittable::Hittable(Sphere sphere){
        shapeType = SHAPE_SPHERE;
        this->sphere = sphere;
    }

    __device__ float Hittable::rayCollide(const Raytracer::Ray ray) const{
        // printf("Inside Hittable: %f", sphere.radius);
        if(shapeType == SHAPE_SPHERE){
            return SphereRayCollide(sphere, ray);
        }
        else{
            return -1;
        }
    }



}