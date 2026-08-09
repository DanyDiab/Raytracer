#pragma once

#include "../RayHits/Ray.cuh"
#include <glm/vec3.hpp>

namespace Raytracer{
    struct Triangle{
        glm::vec3 p1;
        glm::vec3 p2;
        glm::vec3 p3;
    };
}


__device__ float TriangleRayCollide(const Raytracer::Triangle triangle, const Raytracer::Ray ray);

__device__ glm::vec3 TriangleRayNormal(const Raytracer::Triangle triangle, Raytracer::Ray ray);