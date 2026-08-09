#include "../headers/RayHits/Ray.cuh"
#include "../headers/Hittables/Triangle.cuh"
#include <glm/ext/quaternion_common.hpp>
#include <glm/geometric.hpp>
#include <stdio.h>



__device__ glm::vec3 TriangleNormal(const Raytracer::Triangle triangle){
    glm::vec3 AB = triangle.p2 - triangle.p1;
    glm::vec3 AC = triangle.p3 - triangle.p1;

    glm::vec3 ortho = glm::cross(AB, AC);
 
    return glm::normalize(ortho);
}

__device__ glm::vec3 TriangleRayNormal(const Raytracer::Triangle triangle, Raytracer::Ray ray){
    glm::vec3 normal = TriangleNormal(triangle);

    if(glm::dot(normal, ray.dir) > 0.0f){
        normal = -normal;
    }

    return normal;
}

// returns colision distance with plane with the ray
__device__ float PlaneRayCollide(const Raytracer::Triangle triangle, const Raytracer::Ray ray, glm::vec3 normal){
    float denom = glm::dot(ray.dir, normal);

    if(denom == 0.0f){
        // degenerate
        return -1;
    }

    float numerator = glm::dot(triangle.p1, normal) - glm::dot(ray.origin, normal);

    float t = numerator / denom;

    return t;
}


__device__ bool pointInTriangle(const Raytracer::Triangle triangle, glm::vec3 point){
    // get barycentric coordinates

    glm::vec3 triNormal = TriangleNormal(triangle);

    glm::vec3 AB = triangle.p2 - triangle.p1;
    glm::vec3 AC = triangle.p3 - triangle.p1;

    glm::vec3 ABNormal = glm::cross(triNormal, AB);
    glm::vec3 ACNormal = glm::cross(AC, triNormal);


    float bC = glm::dot(ABNormal, point - triangle.p1) / glm::dot(ABNormal, triangle.p3 - triangle.p1);
    float bB = glm::dot(ACNormal, point - triangle.p1) / glm::dot(ACNormal, triangle.p2 - triangle.p1);
    float bA = 1.0f - bB - bC;

    bool valid = bA >= 0.0f && bB >= 0.0f && bC >= 0.0f;

    return valid;
}


__device__ float TriangleRayCollide(const Raytracer::Triangle triangle, const Raytracer::Ray ray){
    glm::vec3 normal = TriangleRayNormal(triangle, ray);

    float planeHitDistance = PlaneRayCollide(triangle, ray, normal);

    if(planeHitDistance <= 0.0f){
        return -1.0f;
    }

    glm::vec3 planeHitPoint = ray.origin + ray.dir * planeHitDistance;

    bool inTriangle = pointInTriangle(triangle, planeHitPoint);

    float dist = inTriangle ? planeHitDistance : -1;

    return dist;
}



