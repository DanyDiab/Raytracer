#include "headers/Sphere.hpp"


Sphere::Sphere(){
    position = glm::vec3(0,0,0);
    radius = 1;
}

Sphere::Sphere(glm::vec3 pos, float radius){
    position = pos;
    this->radius = radius;
}