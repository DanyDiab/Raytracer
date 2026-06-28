#pragma once

#include "Transform.hpp"

class Sphere{
    public:
        Sphere(glm::vec3 pos, float radius);
        // sphere at (0,0,0) with radius 1
        Sphere();
        
        glm::vec3 position;
        float radius;

    private:
};