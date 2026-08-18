#include <algorithm>
#include <cstddef>
#include <glm/geometric.hpp>
#include <vector>
#include <stack>
#include <cstdio>
#include "../headers/Hittables/BVH.cuh"
#include "../headers/Hittables/AABB.cuh"

glm::vec3 getShapeCenter(const Raytracer::Hittable& shape){
    glm::vec3 center;
    switch(shape.shapeType){
        case Raytracer::SHAPE_SPHERE:{
            center = shape.shape.sphere.position;
            break;
        }
        case Raytracer::SHAPE_TRIANGLE:{

            glm::vec3 sum = shape.shape.triangle.p1 + shape.shape.triangle.p2 + shape.shape.triangle.p3;
            center = sum / 3.0f;
            break;
        }
    }
    
    return center;
}

std::vector<Raytracer::Hittable> sortShapes(std::vector<Raytracer::Hittable>& shapes, glm::vec3 axis){
    std::sort(shapes.begin(), shapes.end(), [&axis](const Raytracer::Hittable& a, const Raytracer::Hittable& b){
        glm::vec3 centerA = getShapeCenter(a);
        glm::vec3 centerB = getShapeCenter(b);

        float aScore = glm::length(centerA * axis);
        float bScore = glm::length(centerB * axis);

        return aScore < bScore;
    });

    return shapes;
}

// this needs to be moved to parallel GPU construction
void BVH::BVH::constructBVHRecur(int nodeIndex, std::vector<Raytracer::Hittable>& shapes){

    
    AABB aabb = AABB(shapes);
    
    nodes[nodeIndex].aabb = aabb;

    if(shapes.size() == 1){
        nodes[nodeIndex].childStart = -1.0f;
        nodes[nodeIndex].shape = shapes[0];
        return;
    }

    glm::vec3 axisToSplit = aabb.getLongestAxis();
    
    sortShapes(shapes, axisToSplit);

    std::size_t count = shapes.size();
    std::size_t half = glm::floor(count / 2.0f);

    std::vector<Raytracer::Hittable>::iterator splitPoint = shapes.begin() + half;

    std::vector<Raytracer::Hittable> left(shapes.begin(), splitPoint);
    std::vector<Raytracer::Hittable> right(splitPoint, shapes.end());



    int leftChildIndex = static_cast<int>(nodes.size());
    int rightChildIndex = leftChildIndex + 1;

    nodes[nodeIndex].childStart = leftChildIndex;
    nodes.resize(nodes.size() + 2);

    constructBVHRecur(leftChildIndex, left);
    constructBVHRecur(rightChildIndex, right);
}


BVH::BVH::BVH(std::vector<Raytracer::Hittable> shapes){
    nodes.resize(1);
    constructBVHRecur(0, shapes);
}


__device__ Raytracer::HitRecord BVH::trace(const Raytracer::Ray ray, const BVHNode* nodes){

    int stack[64];
    int stackPtr = 0;

    stack[stackPtr++] = 0;

    Raytracer::HitRecord closestRecord;
    Raytracer::Hittable closestShape;

    closestRecord.hitDistance = INFINITY;

    while(stackPtr > 0){
        BVHNode root = nodes[stack[--stackPtr]];

        if(root.aabb.RayCollide(ray) == -1.0f){
            continue;
        } 
        // trace this shape
        if(root.childStart < 0){
            Raytracer::Hittable shapeHit = root.shape;
            Raytracer::HitRecord record = shapeHit.rayCollide(ray);

            if(record.hitDistance < 0) continue;

            if(record.hitDistance < closestRecord.hitDistance){
                closestShape = shapeHit;
                closestRecord = record;
                closestRecord.mat = shapeHit.mat;
            }
            continue;
        }

        // add left and right child
        stack[stackPtr++] = root.childStart;
        stack[stackPtr++] = root.childStart + 1;
    }

    if(closestRecord.hitDistance < INFINITY){
        closestRecord.normal = closestShape.getShapeNormal(closestRecord.hitDistance, ray);
    }
    else{
        closestRecord.hitDistance = -1.0f;
    }

    return closestRecord;
}

