#pragma once

#include <glm/ext/vector_float3.hpp>
#include "../RayHits/Ray.cuh"

namespace Raytracer{
    struct Plane{
    
        glm::vec3 normal;
        glm::vec3 anchor;
    };

    __device__ float PlaneRayCollide(const Raytracer::Plane plane, const Raytracer::Ray ray);
}

