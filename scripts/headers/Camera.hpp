#pragma once

#include <cuda_runtime.h>
#include "HitRecord.hpp"
#include "Ray.cuh"
#include "Sphere.cuh"
#include "Transform.hpp"
#include <glm/vec3.hpp>
#include <memory>
#include <vector>
#include "Hittable.cuh"

struct ViewportInfo{
    float near;
    float far;
    int width;
    int height;
};

class Camera{
    public:
        Camera(ViewportInfo vi, glm::vec3 pos, glm::quat rot);
        // intilize at 0,0,0, looking down the positive Z (0,0,1)
        Camera(ViewportInfo vi);

        std::unique_ptr<ViewportInfo> viewportInfo;

        glm::vec3 viewportPos;

        void generateRays();
        void shootRays(const std::vector<std::shared_ptr<Raytracer::Hittable>>& objects);
        void launchCollisionKernel(const std::vector<std::shared_ptr<Raytracer::Hittable>>& hittables);

    private:
        Raytracer::Transform transform;
        std::vector<Raytracer::Ray> rays;
        std::vector<Raytracer::HitRecord> hitRecords;
        

};
