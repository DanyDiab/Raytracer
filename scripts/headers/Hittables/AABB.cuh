#pragma once

#include "Hittable.cuh"
#include "Plane.cuh"
#include <vector>

namespace BVH{
    struct Bounds{
        glm::vec3 min;
        glm::vec3 max;
    };

    struct Slab{
        Raytracer::Plane plane1;
        Raytracer::Plane plane2;
    };

    class AABB{
        public:
            AABB(const std::vector<Raytracer::Hittable> hittables);
            Bounds bounds;
            __device__ const float RayCollide(const Raytracer::Ray ray);
            glm::vec3 getLongestAxis();
        private:
            void CreateSlabs();
            
            Slab xSlab;
            Slab ySlab;
            Slab zSlab;
    };
}
