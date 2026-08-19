#pragma once

#include <cuda_runtime.h>
#include "../RayHits/HitRecord.hpp"
#include "../RayHits/Ray.cuh"
#include "../Util/Transform.hpp"
#include <glm/vec3.hpp>
#include <memory>
#include <vector>
#include "../Hittables/Hittable.cuh"

struct ViewportInfo{
    float near;
    float far;
    int width;
    int height;
};

class Camera{
    public:
        Camera(const ViewportInfo vi, glm::vec3 pos, glm::quat rot);
        // intilize at 0,0,0, looking down the positive Z (0,0,1)
        Camera(const ViewportInfo vi);

        ViewportInfo viewportInfo;

        glm::vec3 viewportPos;
        std::vector<glm::vec3> Render(const std::vector<Raytracer::Hittable> objects, int renderingFlags);

    private:
        Raytracer::Transform transform;
        std::vector<Raytracer::Ray> rays;
        std::vector<Raytracer::HitRecord> hitRecords;
        

};
