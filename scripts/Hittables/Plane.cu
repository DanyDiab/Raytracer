#include "../headers/Hittables/Plane.cuh"

// returns colision distance with plane with the ray
__device__ float Raytracer::PlaneRayCollide(const Raytracer::Plane plane, const Raytracer::Ray ray){

    float denom = glm::dot(ray.dir, plane.normal);

    if(denom == 0.0f){
        // degenerate, this is parallel to the plane
        return -1;
    }

    float numerator = glm::dot(plane.anchor, plane.normal) - glm::dot(ray.origin, plane.normal);

    float t = numerator / denom;

    return t;
}