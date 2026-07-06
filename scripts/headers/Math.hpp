#pragma once

#include <glm/ext/vector_float3.hpp>

namespace Raytracer{
    glm::vec3 randomUnitVec();

    // returns a random unit vector that is on the same hemisphere as the normal vector (used for diffuse)
    glm::vec3 randomUnitVecSameHemisphere(glm::vec3 normal);
}