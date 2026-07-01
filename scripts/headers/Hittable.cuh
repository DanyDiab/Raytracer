#pragma once

#include "Ray.hpp"
#include "Material.hpp"
#include "Transform.hpp"
#include "Sphere.cuh"
#include <cuda_runtime.h>

namespace Raytracer{
    enum ShapeType{
        SHAPE_SPHERE
    };

    union Geometry{
        Sphere sphere;
        
        __host__ Geometry() {}
        
        __host__ ~Geometry() {}
    };

    class Hittable{
        public:
            __host__ Hittable();
            __host__ Hittable(Sphere sphere);

            __host__ ~Hittable();
            Raytracer::Geometry Geometry;
            Material mat;
             __device__ float rayCollide(const Raytracer::Ray ray) const;
            ShapeType shapeType;


        private:
    };
}