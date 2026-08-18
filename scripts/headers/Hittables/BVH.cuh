#pragma once


#include "Hittable.cuh"
#include "AABB.cuh"
#include <vector>

namespace BVH{
    struct BVHNode{
        // set to -1 if a leaf
        int childStart{-1};
        BVH::AABB aabb;
        // this is set if its a leaf
        Raytracer::Hittable shape;

        __device__ __host__ BVHNode() = default;
        __device__ __host__ ~BVHNode() = default;
    };

    
    class BVH {
        public:
            BVH(std::vector<Raytracer::Hittable> shapes);
            std::vector<BVHNode> nodes;

        private:
            void constructBVHRecur(int nodeIndex, std::vector<Raytracer::Hittable>& shapes);

    };

    // NOTE that this parameter is the GPU Memory nodes
    __device__ Raytracer::HitRecord trace(const Raytracer::Ray ray, const BVHNode* nodes);

}