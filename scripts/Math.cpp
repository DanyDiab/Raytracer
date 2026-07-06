#include "./headers/Math.hpp"
#include <glm/gtc/random.hpp>

glm::vec3 Raytracer::randomUnitVec(){
    return glm::sphericalRand(1.0);
}


glm::vec3 Raytracer::randomUnitVecSameHemisphere(glm::vec3 normal){
    glm::vec3 rand = randomUnitVec();

    if(glm::dot(normal, rand) < 0.0f){
        return -rand;
    }
    return rand;
}