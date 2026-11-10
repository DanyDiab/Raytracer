#pragma once

#include <curand_kernel.h>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>

namespace PRNG{
    __global__ void initRandStates(unsigned long long currTime, curandState_t* currStates, int numStates);

    __device__ float randFloat(curandState_t* state);

    __device__ glm::vec3 randomUnitVec(curandState_t* state);

    __device__ glm::vec3 randomUnitVecSameHemisphere(const glm::vec3& normal, curandState_t* state);
}

