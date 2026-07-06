#pragma once

#include "Ray.hpp"
#include <glm/vec3.hpp>

namespace Raytracer{
    struct Sphere{
        float radius;
        glm::vec3 position;
    };
}


__device__ float SphereRayCollide(const Raytracer::Sphere sphere, const Raytracer::Ray ray);





