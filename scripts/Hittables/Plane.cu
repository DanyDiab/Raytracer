#include "../headers/Hittables/Plane.cuh"
#include <cmath>

// returns colision distance with plane with the ray
__device__ float Raytracer::PlaneRayCollide(const Raytracer::Plane plane, const Raytracer::Ray ray){

    float denom = glm::dot(ray.dir, plane.normal);

    if(denom == 0.0f){
        // degenerate, this is parallel to the plane
        float distanceToPlane = glm::dot(plane.anchor - ray.origin, plane.normal);
        return distanceToPlane >= 0.0f ? -INFINITY : INFINITY;
    }

    float numerator = glm::dot(plane.anchor, plane.normal) - glm::dot(ray.origin, plane.normal);

    return numerator / denom;
}