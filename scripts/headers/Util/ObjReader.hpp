#pragma once


#include <string>
#include <vector>
#include "../Hittables/Triangle.cuh"

namespace FileOps{
    std::vector<Raytracer::Triangle> ReadInObj(std::string filePath);
}
