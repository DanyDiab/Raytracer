#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Raytracer{
    struct Transform{
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;

        glm::mat4x4 GetModelMatrix() const{
            glm::mat4 translateMat = glm::translate(glm::mat4(1), position);
            glm::mat4 rotationMat = glm::mat4_cast(rotation);
            glm::mat4 scaleMat = glm::scale(glm::mat4(1), scale);

            return translateMat * rotationMat * scaleMat;
        }
    };
}