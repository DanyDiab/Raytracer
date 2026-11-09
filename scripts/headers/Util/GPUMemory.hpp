#pragma once

#include "../Hittables/Hittable.cuh"
#include <curand_kernel.h>

struct GPUMemory{
    Raytracer::Hittable* hittable;
    glm::vec3* colors;
    curandState_t* prngStates;
};