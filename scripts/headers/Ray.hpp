#pragma once

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <iostream>

namespace Raytracer{
    struct Ray{
        glm::vec3 origin;
        // normalized
        glm::vec3 dir;
        
        Ray() = default;
        Ray(glm::vec3 origin, glm::vec3 dir){
            this->origin = origin;
            float mag = glm::length(dir);
            // normalize
            
            if(mag <= 0.0f){
                std::cerr << "RAY CONSTRUCTOR: cannot normalize a vector with magnitude 0";
                this->dir = glm::vec3(0.0f, 0.0f, -1.0f);
                return;
            }
            this->dir = dir / mag;
        }
    };
}
