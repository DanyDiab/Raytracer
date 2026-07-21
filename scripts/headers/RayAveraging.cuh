#pragma once



#include "HitRecord.hpp"
#include <glm/ext/vector_float3.hpp>

struct ColorAverage{
    glm::vec3 color;
    int count;
};

__host__ void rayAverage(ColorAverage* colors, Raytracer::HitRecord* records, int numRecords, int numPixels, int raysPerPixel);