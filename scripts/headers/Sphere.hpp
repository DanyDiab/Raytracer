#pragma once

#include "Ray.hpp"
#include "Transform.hpp"

class Sphere{
    public:
        Sphere(glm::vec3 pos, glm::quat rot, glm::vec3 scale);
        // sphere at (0,0,0) with radius 1, with no rotation
        Sphere();
        
        Raytracer::Transform transform;

    private:
};