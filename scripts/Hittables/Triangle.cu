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
};

// returns colision distance with plane with the ray
__device__ float PlaneRayCollide(const Raytracer::Triangle triangle, const Raytracer::Ray ray, glm::vec3 normal){
    float denom = glm::dot(ray.dir, normal);

    if(denom == 0.0f){
        // degenerate
        return -1;
    }

    float numerator = glm::dot(triangle.p1, normal) * glm::dot(ray.origin, normal);

    float t = numerator / denom;

    return t;
}

__device__ float getTriangleArea(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    glm::vec3 s1 = p2 - p1;
    glm::vec3 s2 = p3 - p1;
    return 0.5f * glm::length(glm::cross(s1, s2));
}

__device__ float TriangleRayCollide(const Raytracer::Triangle triangle, const Raytracer::Ray ray){
    glm::vec3 normal = TriangleNormal(triangle);

    float planeHitDistance = PlaneRayCollide(triangle, ray, normal);

    // get barycentric coordinates
    if(planeHitDistance == -1.0f){
        return -1.0f;
    }

    glm::vec3 planeHitPoint = ray.origin + ray.dir * planeHitDistance;

    float totalArea = getTriangleArea(triangle.p1, triangle.p2, triangle.p3);

    float tAArea = getTriangleArea(planeHitPoint, triangle.p2, triangle.p3);
    float tBArea = getTriangleArea(triangle.p1, planeHitPoint, triangle.p3);
    float tCArea = getTriangleArea(triangle.p1, triangle.p2, planeHitPoint);

    float bA = tAArea / totalArea;
    float bB = tBArea / totalArea;
    float bC = tCArea / totalArea;

    float totalContirub = bA + bB + bC;

    printf("%F\n", totalContirub);
    if(glm::abs(totalContirub - 1.0) >= .01f || bA < 0.0f || bB < 0.0f || bC < 0.0f){
        return -1;
    }

    return planeHitDistance;
}



