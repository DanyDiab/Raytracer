#include <cmath>
#include <cuda_runtime.h>
#include <cstdio>

#include "../headers/RayHits/Ray.cuh"
#include "../headers/Util/Transform.hpp"
#include "../headers/Hittables/Sphere.cuh"
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

__device__ glm::vec3 SphereRayNormal(const Raytracer::Sphere sphere, const Raytracer::Ray ray, const float distance){

    glm::vec3 hitPoint = ray.origin + (ray.dir * distance);
    glm::vec3 sphereToRay = hitPoint - sphere.position;

    return glm::normalize(sphereToRay);

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

    float tMin = 0.001f;

    float sqrtdiscriminant = ::sqrt(discriminant);
    
    // check 1st intersection
    float t1 = (-b - sqrtdiscriminant) / (2.0f * a);
    if (t1 >= tMin) {
        return t1;
    }

    // check 2nd intersection
    float t2 = (-b + sqrtdiscriminant) / (2.0f * a);
    if (t2 >= tMin) {
        return t2;
    }
    
    return -1.0f;

}