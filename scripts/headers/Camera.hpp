#pragma once

#include "Transform.hpp"

class Camera{
    public:
        Camera camera(glm::vec3 pos, glm::quat rot);
        // intilize at 0,0,0, looking down the positive Z (0,0,1)
        Camera camera();
    private:
        Raytracer::Transform position;
        

};