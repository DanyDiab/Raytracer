#pragma once

#include "Ray.hpp"
#include <glm/vec3.hpp>

namespace Raytracer{
    struct Sphere{
        float radius;
        glm::vec3 position;
    };
}


inline __device__ float SphereRayCollide(const Raytracer::Sphere sphere, const Raytracer::Ray ray){
    glm::vec3 offset = sphere.position - ray.origin;

    float a = glm::dot(ray.dir, ray.dir);
    float b = -2.0f * glm::dot(ray.dir, offset);
    float c = glm::dot(offset, offset) - (sphere.radius * sphere.radius);

    float discriminant = b * b - 4.0f * a * c;
    
    if (discriminant < 0.0f) {
        return -1.0f;
    }

    float t = (-b - ::sqrt(discriminant)) / (2.0f * a);
    
    if (t < 0.0f) {
        return -1.0f;
    }
    
    return t;
}


