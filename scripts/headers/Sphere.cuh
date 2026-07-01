#pragma once

#include "Ray.hpp"
#include <glm/ext/vector_float3.hpp>

struct Sphere{
    float radius;
    glm::vec3 position;

    Sphere(glm::vec3 position, float radius);
};

__device__ float SphereRayCollide(const Sphere sphere, const Raytracer::Ray ray);


