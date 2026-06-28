#pragma once

#include "Ray.hpp"
#include "Sphere.hpp"
#include "Transform.hpp"
#include <glm/vec3.hpp>
#include <memory>
#include <vector>


struct ViewportInfo{
    float near;
    float far;
    float width;
    float height;
};

class Camera{
    public:
        Camera(ViewportInfo vi, glm::vec3 pos, glm::quat rot);
        // intilize at 0,0,0, looking down the positive Z (0,0,1)
        Camera(ViewportInfo vi);

        std::unique_ptr<ViewportInfo> viewportInfo;

        glm::vec3 viewportPos;

        void generateRays();
        void shootRays(Sphere sphere);
    private:
        Raytracer::Transform transform;
        std::vector<Raytracer::Ray> rays;
        

};
