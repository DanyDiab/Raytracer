#pragma once

#include "Ray.hpp"
#include "Material.hpp"
#include "Transform.hpp"
#include "Sphere.cuh"

namespace Raytracer{
    enum ShapeType{
        SHAPE_SPHERE
    };

    union Geometry{
        Sphere sphere;
    };

    class Hittable{
        public:
            __host__ __device__ Hittable() = default;
            __host__ __device__ Hittable(Sphere sphere);

            __host__ __device__ ~Hittable() = default;

            Sphere sphere;

            Material mat;
            inline __device__ float rayCollide(const Raytracer::Ray ray) const{
                if(shapeType == SHAPE_SPHERE){
                    return SphereRayCollide(sphere, ray);
                }
                else{
                    return -1;
                }
            };
            
            ShapeType shapeType;


        private:
    };
}