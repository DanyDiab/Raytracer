#pragma once

#include <glm/ext/vector_float3.hpp>
namespace Raytracer{
    struct Material{
        // essentially just color (not really tho)
        glm::vec3 albedo;
        // 1 = fully metallic 0 = diffuse 
        float metallic;
        float roughness;
    };
}
