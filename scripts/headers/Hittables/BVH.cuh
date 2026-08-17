#pragma once


#include "Hittable.cuh"
#include "AABB.cuh"
#include <vector>

namespace BVH{
    struct BVHNode{
        // set to -1 if a leaf
        int childStart;
        BVH::AABB aabb;
        // this is set if its a leaf
        Raytracer::Hittable shape;
    };

    
    class BVH {
        public:
            BVH(std::vector<Raytracer::Hittable> shapes);
            __device__ std::vector<Raytracer::Hittable> GetCandidateShapes(Raytracer::Ray ray);
            std::vector<BVHNode> nodes;

        private:
            void constructBVHRecur(std::vector<Raytracer::Hittable>& shapes);
    };
}