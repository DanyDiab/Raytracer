#pragma once

#include "HitRecord.hpp"
#include "Ray.cuh"
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
            __device__  Raytracer::HitRecord rayCollide(const Raytracer::Ray ray) const;

            
            ShapeType shapeType;


        private:
    };
}