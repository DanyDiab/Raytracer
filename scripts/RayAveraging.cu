#include <glm/ext/vector_float3.hpp>

__global__ void AverageRayColors(glm::vec3* colors, int numColors, int samplesPerPixel){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);
    
    if(index < numColors){
        colors[index] /= samplesPerPixel;
    }
}