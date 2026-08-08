#pragma once

#include "../RayHits/HitRecord.hpp"
#include "../RayHits/Ray.cuh"
#include "Material.hpp"
#include "../Util/Transform.hpp"
#include "../Hittables/Sphere.cuh"
#include "../Hittables/Triangle.cuh"


namespace Raytracer{
    enum ShapeType{
        SHAPE_SPHERE,
        SHAPE_TRIANGLE
    };

    union Geometry{
        Sphere sphere;
        Triangle triangle;
    };

    class Hittable{
        public:
            __host__ __device__ Hittable() = default;
            __host__ __device__ Hittable(Sphere sphere);
            __host__ __device__ Hittable(Triangle triangle);

            __host__ __device__ ~Hittable() = default;

            Geometry shape;

            Material mat;
            __device__  Raytracer::HitRecord rayCollide(const Raytracer::Ray ray) const;

            
            ShapeType shapeType;


        private:
    };
}