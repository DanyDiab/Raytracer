#pragma once


#include <glm/ext/vector_float3.hpp>
inline __device__ void printVec(glm::vec3 vecToPrint){
    printf("%f X | %f Y | %f Z\n", vecToPrint.x,vecToPrint.y,vecToPrint.z);
}