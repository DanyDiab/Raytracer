#include "headers/HitRecord.hpp"
#include <glm/ext/vector_float3.hpp>
#include "headers/RayAveraging.cuh"


__host__ void rayAverage(ColorAverage* colors, Raytracer::HitRecord* records, int numRecords, int numPixels, int raysPerPixel){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);

    if(index < numRecords){
        int pixelIndex = index / numPixels;

        ColorAverage ca = colors[pixelIndex];
        ca.color += records[index].color;

        if(ca.count == raysPerPixel){
            ca.color /= raysPerPixel;
        }

        ca.count++;
    }
}