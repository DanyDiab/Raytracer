#pragma once

#include <glm/ext/vector_float3.hpp>

namespace Raytracer{
    __device__ glm::vec3 randomUnitVec(unsigned int& seed);


    // returns a random unit vector that is on the same hemisphere as the normal vector (used for diffuse)
    __device__ glm::vec3 randomUnitVecSameHemisphere(glm::vec3 normal, unsigned int& seed);

}