#include <curand_kernel.h>
#include <curand_uniform.h>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include "../headers/Util/PRNG.cuh"


__device__ void initRandState(int index, unsigned long long currTime, curandState_t* currState) {

    if (currState == nullptr) {
        return;
    }

    curand_init(currTime, static_cast<unsigned long long>(index), 0, currState);
}

__global__ void PRNG::initRandStates(unsigned long long currTime, curandState_t* currStates, int numStates){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);

	if(index < numStates){
		curandState_t* currState = &currStates[index];
		initRandState(index, currTime,currState);
	}
}

__device__ float PRNG::randFloat(curandState_t* state) {
    return curand_uniform(state);
}

__device__ glm::vec3 PRNG::randomUnitVec(curandState_t* state) {
    float u1 = randFloat(state);
    float u2 = randFloat(state);

    float z = 1.0f - (2.0f * u1);
    float r = sqrtf(fmaxf(0.0f, 1.0f - (z * z)));
    float phi = 6.283185307179586f * u2;

    return glm::vec3(r * cosf(phi), r * sinf(phi), z);
}


__device__ glm::vec3 PRNG::randomUnitVecSameHemisphere(const glm::vec3& normal, curandState_t* state) {
    glm::vec3 randVec = randomUnitVec(state);

    if (glm::dot(normal, randVec) < 0.0f) {
        return -randVec;
    }

    return randVec;
}