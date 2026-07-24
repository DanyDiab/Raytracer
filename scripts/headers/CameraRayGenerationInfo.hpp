#pragma once

#include <glm/ext/vector_float3.hpp>

struct CameraRayGenerationInfo{
    glm::vec3 forward;
    glm::vec3 right;
     glm::vec3 up;
     glm::vec3 camPos;
     float leftOffset;
     float botOffset;
     int width;
     int height;
};