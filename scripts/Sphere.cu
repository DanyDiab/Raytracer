#include <cuda_runtime.h>
#include <cstdio>

#include "headers/Ray.cuh"
#include "headers/Transform.hpp"
#include "headers/Sphere.cuh"
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

__device__ glm::vec3 SphereRayNormal(const Raytracer::Sphere sphere, const Raytracer::Ray ray, const float distance){
    return glm::normalize(ray.origin + (ray.dir * distance));
}

__device__ float SphereRayCollide(const Raytracer::Sphere sphere, const Raytracer::Ray ray){

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