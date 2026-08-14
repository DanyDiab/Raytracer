#pragma once


#include <glm/ext/vector_float3.hpp>
#include <string>
#include <vector>

namespace FileOps{
    void writeColorsToPPM(std::vector<glm::vec3> colors, int height, int width, std::string filePath);
}

