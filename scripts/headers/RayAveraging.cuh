#pragma once

#include <glm/ext/vector_float3.hpp>

__global__ void AverageRayColors(glm::vec3* colors, int numColors, int samplesPerPixel);
