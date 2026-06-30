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


float Sphere::rayCollide(Raytracer::Ray ray){
    glm::vec3 offset = transform.position - ray.origin;

    float a = glm::dot(ray.dir, ray.dir);
    float b = -2.0f * glm::dot(ray.dir, offset);
    float c = glm::dot(offset,offset) - (transform.scale.x * transform.scale.x);

    float discriminant = b * b - 4 * a * c;
    // missed
    if(discriminant < 0.0f){
        return -1.0f;
    }

    float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
    
    // the roots are all behind this ray
    if(t < 0.0f) return -1.0f;
    
    return t;
}