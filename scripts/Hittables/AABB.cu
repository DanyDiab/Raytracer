#include "../headers/Hittables/AABB.cuh"
#include <iostream>
#include <vector>

BVH::Bounds getBounds(Raytracer::Sphere sphere){
    BVH::Bounds bounds;

    glm::vec3 pos = sphere.position;
    float radius = sphere.radius;

    float minX = pos.x - radius;
    float maxX = pos.x + radius;

    float minY = pos.y - radius;
    float maxY = pos.y + radius;

    float minZ = pos.z - radius;
    float maxZ = pos.z + radius;

    glm::vec3 min = glm::vec3(minX, minY, minZ);
    glm::vec3 max = glm::vec3(maxX, maxY, maxZ);

    bounds.min = min;
    bounds.max = max;

    return bounds;
}

BVH::Bounds getBounds(Raytracer::Triangle triangle){
    BVH::Bounds bounds;
    
    glm::vec3 p1 = triangle.p1;
    glm::vec3 p2 = triangle.p2;
    glm::vec3 p3 = triangle.p3;


    float minX = std::min(p1.x, std::min(p2.x, p3.x));
    float minY = std::min(p1.y, std::min(p2.y, p3.y));
    float minZ = std::min(p1.z, std::min(p2.z, p3.z));

    
    float maxX = std::max(p1.x, std::max(p2.x, p3.x));
    float maxY = std::max(p1.y, std::max(p2.y, p3.y));
    float maxZ = std::max(p1.z, std::max(p2.z, p3.z));

    glm::vec3 min = glm::vec3(minX, minY, minZ);
    glm::vec3 max = glm::vec3(maxX, maxY, maxZ);

    bounds.min = min;
    bounds.max = max;

    return bounds;
}

void updateEntireBounds(BVH::Bounds* entireBounds, BVH::Bounds* newBoundToInclude){
    entireBounds->min.x = std::min(entireBounds->min.x, newBoundToInclude->min.x);
    entireBounds->min.y = std::min(entireBounds->min.y, newBoundToInclude->min.y);
    entireBounds->min.z = std::min(entireBounds->min.z, newBoundToInclude->min.z);

    entireBounds->max.x = std::max(entireBounds->max.x, newBoundToInclude->max.x);
    entireBounds->max.y = std::max(entireBounds->max.y, newBoundToInclude->max.y);
    entireBounds->max.z = std::max(entireBounds->max.z, newBoundToInclude->max.z);
}

BVH::AABB::AABB(std::vector<Raytracer::Hittable> shapes){
    BVH::Bounds bounds;
    bounds.min = glm::vec3(1 >> 31);
    bounds.max = glm::vec3(1 << 31);

    for(const auto& hit : shapes){
        BVH::Bounds shapeBounds;
        switch(hit.shapeType){
            case Raytracer::SHAPE_SPHERE:{
                Raytracer::Sphere sphere = hit.shape.sphere;
                shapeBounds = getBounds(sphere);
                break;
            };
            case Raytracer::SHAPE_TRIANGLE:{
                Raytracer::Triangle tri = hit.shape.triangle;
                shapeBounds = getBounds(tri);
                break;
            }
            default:{
                std::cerr << "ERROR: HOW DID WE GET HERE? UNKNOWN SHAPE TYPE IN AABB CONSTRUCTION";
            }
        }
        updateEntireBounds(&bounds, &shapeBounds);
    }

    this->bounds = bounds;

}


