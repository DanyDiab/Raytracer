#include "../headers/RayHits/Ray.cuh"
#include "../headers/Hittables/Triangle.cuh"
#include "../headers/Hittables/Plane.cuh"

#include <glm/ext/quaternion_common.hpp>
#include <glm/geometric.hpp>
#include <stdio.h>




inline __device__ glm::vec3 TriangleFaceNormal(const Raytracer::Triangle triangle){
    glm::vec3 AB = triangle.p2 - triangle.p1;
    glm::vec3 AC = triangle.p3 - triangle.p1;

    glm::vec3 ortho = glm::cross(AB, AC);
 
    return glm::normalize(ortho);
}

inline __device__ Raytracer::BACoords GetBaryCentricCoords(const Raytracer::Triangle triangle, glm::vec3 point){
    // get barycentric coordinates

    glm::vec3 triNormal = TriangleFaceNormal(triangle);

    glm::vec3 AB = triangle.p2 - triangle.p1;
    glm::vec3 AC = triangle.p3 - triangle.p1;

    glm::vec3 ABNormal = glm::cross(triNormal, AB);
    glm::vec3 ACNormal = glm::cross(AC, triNormal);

    float bC = glm::dot(ABNormal, point - triangle.p1) / glm::dot(ABNormal, triangle.p3 - triangle.p1);
    float bB = glm::dot(ACNormal, point - triangle.p1) / glm::dot(ACNormal, triangle.p2 - triangle.p1);
    float bA = 1.0f - bB - bC;

    Raytracer::BACoords baCoords{
        .bA = bA,
        .bB = bB,
        .bC = bC
    };

    return baCoords;
}

inline __device__ glm::vec3 TriangleVertexRayNormal(const Raytracer::Triangle triangle, const Raytracer::Ray ray, const float distance){
    glm::vec3 hitPoint = ray.origin + (ray.dir * distance);

    Raytracer::BACoords baCoords = GetBaryCentricCoords(triangle, hitPoint);

    glm::vec3 aContib = baCoords.bA * triangle.p1Normal;
    glm::vec3 bContib = baCoords.bB * triangle.p2Normal;
    glm::vec3 cContib = baCoords.bC * triangle.p3Normal;

    glm::vec3 vNorm = aContib + bContib + cContib;

    return vNorm;
}


inline __device__ glm::vec3 TriangleFaceRayNormal(const Raytracer::Triangle triangle, Raytracer::Ray ray){

    glm::vec3 normal = TriangleFaceNormal(triangle);

    if(glm::dot(normal, ray.dir) > 0.0f){
        normal = -normal;
    }

    return normal;
}

// routes between either face or vertex normals, depending on if they are set or not
__device__ glm::vec3 TriangleNormal(const Raytracer::Triangle triangle, const Raytracer::Ray ray, const float distance){

    bool useFaceNorms = triangle.p1Normal == glm::vec3(0);
    glm::vec3 normal = useFaceNorms ? TriangleFaceRayNormal(triangle, ray) : TriangleVertexRayNormal(triangle, ray, distance);

    return normal;
}


inline __device__ bool pointInTriangle(const Raytracer::Triangle triangle, glm::vec3 point){

    Raytracer::BACoords coords = GetBaryCentricCoords(triangle, point);
    bool valid = coords.bA >= 0.0f && coords.bB >= 0.0f && coords.bC >= 0.0f;

    return valid;
}


__device__ float TriangleRayCollide(const Raytracer::Triangle triangle, const Raytracer::Ray ray){
    glm::vec3 normal = TriangleFaceRayNormal(triangle, ray);

    Raytracer::Plane plane{
        .normal = normal,
        .anchor = triangle.p1
    };

    float planeHitDistance = Raytracer::PlaneRayCollide(plane, ray);

    if(planeHitDistance <= 0.0f){
        return -1.0f;
    }

    glm::vec3 planeHitPoint = ray.origin + ray.dir * planeHitDistance;

    bool inTriangle = pointInTriangle(triangle, planeHitPoint);

    float dist = inTriangle ? planeHitDistance : -1;

    return dist;
}



