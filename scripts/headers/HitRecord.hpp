#pragma once

#include "Ray.hpp"

namespace Raytracer{
    struct HitRecord{

        float hitDistance;
        Raytracer::Ray* ray;
    };
}