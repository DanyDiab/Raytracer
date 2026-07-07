#pragma once

#include <cuda_runtime.h>
#include <glm/ext/vector_float3.hpp>

namespace Raytracer{

    class Ray;
    class Hittable;


    struct HitRecord{
        float hitDistance;
        glm::vec3 normal;
        glm::vec3 color;

    };
};