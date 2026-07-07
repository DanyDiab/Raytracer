#include "./headers/Math.cuh"
#include <glm/gtc/random.hpp>


// thanks CHAT
__device__ float randomFloat(unsigned int& seed) {
    seed = seed * 747796405 + 2891336453;
    unsigned int result = ((seed >> ((seed >> 28) + 4)) ^ seed) * 277803737;
    result = (result >> 22) ^ result;
    return result / 4294967295.0f;
}

// thanks CHAT
__device__ glm::vec3 Raytracer::randomUnitVec(unsigned int& seed) {
    float u1 = randomFloat(seed);
    float u2 = randomFloat(seed);

    float z = 1.0f - (2.0f * u1);
    float r = sqrtf(fmaxf(0.0f, 1.0f - (z * z)));
    float phi = 2.0f * 3.1415926535f * u2;

    return glm::vec3(r * cosf(phi), r * sinf(phi), z);
}

// my code again
__device__ glm::vec3 Raytracer::randomUnitVecSameHemisphere(glm::vec3 normal, unsigned int& seed) {
    glm::vec3 randVec = randomUnitVec(seed);

    if (glm::dot(normal, randVec) >= 0.0f) {
        return randVec;
    }
    
    return -randVec;
}