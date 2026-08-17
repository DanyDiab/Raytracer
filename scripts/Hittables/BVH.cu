#include <algorithm>
#include <cstddef>
#include <glm/geometric.hpp>
#include <vector>
#include <stack>

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
void BVH::BVH::constructBVHRecur(std::vector<Raytracer::Hittable>& shapes){
    AABB aabb = AABB(shapes);
    
    if(shapes.size() == 1){
        BVHNode leaf{
            .childStart = -1,
            .aabb = aabb,
            .shape = shapes.at(0),
        };

        nodes.push_back(leaf);
        return;
    }

    glm::vec3 axisToSplit = aabb.getLongestAxis();
    
    sortShapes(shapes, axisToSplit);

    std::size_t count = shapes.size();
    std::size_t half = glm::floor(count / 2.0f);

    std::vector<Raytracer::Hittable>::iterator splitPoint = shapes.begin() + half;

    std::vector<Raytracer::Hittable> left(shapes.begin(), splitPoint);
    std::vector<Raytracer::Hittable> right(splitPoint, shapes.end());


    BVHNode rootNode{
        .childStart = static_cast<int>((nodes.size() - 1)),
        .aabb = aabb
    };

    nodes.push_back(rootNode);

    constructBVHRecur(left);
    constructBVHRecur(right);
}


BVH::BVH::BVH(std::vector<Raytracer::Hittable> shapes){
    constructBVHRecur(shapes);
}
