#pragma once

#include <cuda_runtime.h>
#include "Hittable.cuh"
#include "Ray.hpp"

namespace Raytracer{
    struct HitRecord{
        float hitDistance;
        bool hit;
        Raytracer::Ray ray;
        Raytracer::Hittable shape;
    };
}