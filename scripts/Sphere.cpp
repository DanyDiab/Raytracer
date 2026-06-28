#include "headers/Sphere.hpp"
#include "headers/Transform.hpp"


Sphere::Sphere(){
    transform = {
        .position = glm::vec3(0,0,0),
        .rotation = glm::quat(glm::vec3(0,0,0)),
        .scale  = glm::vec3(1,1,1)
    };

    
}

Sphere::Sphere(glm::vec3 pos, glm::quat rot, glm::vec3 scale){
    transform = {
        .position = pos,
        .rotation = rot,
        .scale  = scale
    };
}