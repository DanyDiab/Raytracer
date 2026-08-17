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

// call after create bounds
void BVH::AABB::CreateSlabs(){
    glm::vec3 minAnchor = bounds.min;
    glm::vec3 maxAnchor = bounds.max; 

    glm::vec3 xNorm = glm::vec3(1.0f,0.0f,0.0f);
    glm::vec3 yNorm = glm::vec3(0.0f,1.0f,0.0f);
    glm::vec3 zNorm = glm::vec3(0.0f,0.0f,1.0f);

    Raytracer::Plane xPlane1{
        .normal = xNorm,
        .anchor = minAnchor
    };
    Raytracer::Plane xPlane2{
        .normal = xNorm,
        .anchor = maxAnchor
    };

    Raytracer::Plane yPlane1{
        .normal = yNorm,
        .anchor = minAnchor
    };
    Raytracer::Plane yPlane2{
        .normal = yNorm,
        .anchor = maxAnchor
    };


    Raytracer::Plane zPlane1{
        .normal = zNorm,
        .anchor = minAnchor
    };
    Raytracer::Plane zPlane2{
        .normal = zNorm,
        .anchor = maxAnchor
    };


    Slab xSlab{
        .plane1 = xPlane1,
        .plane2 = xPlane2
    };

    Slab ySlab{
        .plane1 = yPlane1,
        .plane2 = yPlane2
    };

    Slab zSlab{
        .plane1 = zPlane1,
        .plane2 = zPlane2
    };

    this->xSlab = xSlab;
    this->ySlab = ySlab;
    this->zSlab = zSlab;
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

// make x = y and y = x
inline __device__ void swapValues(float* x, float* y){
    float* temp = x;
    x = y;
    y = temp;
}

__device__ const float BVH::AABB::RayCollide(const Raytracer::Ray ray){
    float tMinX = Raytracer::PlaneRayCollide(xSlab.plane1, ray);

    if(tMinX == -1.0f){
        return -1.0f;
    }

    float tMinY = Raytracer::PlaneRayCollide(ySlab.plane1, ray);

    if(tMinY == -1.0f){
        return -1.0f;
    }

    float tMinZ = Raytracer::PlaneRayCollide(zSlab.plane1, ray);

    if(tMinZ == -1.0f){
        return -1.0f;
    }

    // we know X,Y, and Z have valid hit points now
    float tMaxX = Raytracer::PlaneRayCollide(xSlab.plane2, ray);
    float tMaxY = Raytracer::PlaneRayCollide(ySlab.plane2, ray);
    float tMaxZ = Raytracer::PlaneRayCollide(zSlab.plane2, ray);

    // ensure that the mins are <= the maxs
    if(tMaxX < tMinX){
        swapValues(&tMinX, &tMaxX);
    }
    if(tMaxY < tMinY){
        swapValues(&tMinY, &tMaxY);
    }
    if(tMaxZ < tMinZ){
        swapValues(&tMinZ, &tMaxZ);
    }
    
    // now check if the intervals overlap
    float enterPoint = glm::min(glm::min(tMinX, tMinY), tMinZ);
    float exitPoint = glm::max(glm::max(tMaxX, tMaxY), tMaxZ);
    
    float hit = enterPoint <= exitPoint ? enterPoint : exitPoint;
    return hit;
}

glm::vec3 BVH::AABB::getLongestAxis(){
    float xLen = bounds.max.x - bounds.min.x;
    float yLen = bounds.max.y - bounds.min.y;
    float zLen = bounds.max.z - bounds.min.z;

    if(xLen > yLen && xLen > zLen){
        return glm::vec3(1.0f,0.0f,0.0f);
    }
    else if(yLen > xLen && yLen > zLen){
        return glm::vec3(0.0f, 1.0f, 0.0f);
    }
    else{
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }

}

