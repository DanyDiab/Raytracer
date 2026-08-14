#pragma once

#include "Hittable.cuh"
#include <vector>

namespace BVH{
    struct Bounds{
        glm::vec3 min;
        glm::vec3 max;
    };

    class AABB{
        public:
            AABB(const std::vector<Raytracer::Hittable> hittables);
            Bounds bounds;
            __device__ const float RayCollide(const Raytracer::Ray ray);
    };
}
