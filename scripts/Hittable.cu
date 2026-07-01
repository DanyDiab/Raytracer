#include "headers/Hittable.cuh"
#include "headers/Sphere.cuh"
#include <cuda_runtime.h>

namespace Raytracer {
    Hittable::~Hittable() {}

    Hittable::Hittable(Sphere sphere){
        if(dynamic_cast<Sphere*>(&sphere) != nullptr){
            shapeType = SHAPE_SPHERE;
        }
        new (&Geometry.sphere) Sphere(sphere);
        
    }

    __device__ float Hittable::rayCollide(const Raytracer::Ray* ray) const{
        if(shapeType == SHAPE_SPHERE){
            return SphereRayCollide(Geometry.sphere, ray);
        }
        else{
            return -1;
        }
    };

}