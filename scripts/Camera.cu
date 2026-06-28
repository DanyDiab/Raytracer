#include "headers/Camera.hpp"
#include "headers/Sphere.hpp"
#include "headers/Transform.hpp"
#include <cmath>
#include <cuda_runtime_api.h>
#include <glm/common.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

#include "headers/Ray.hpp"
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cuda/std/cmath>

Camera::Camera(ViewportInfo vi) {
    transform = {
        .position = glm::vec3(0,0,0),
        .rotation = glm::quat(1,0,0,0),
        .scale = glm::vec3(1,1,1)
    };

    viewportInfo = std::make_unique<ViewportInfo>(vi);
}

Camera::Camera(ViewportInfo vi, glm::vec3 pos, glm::quat rot){
    transform = {
        .position = pos,
        .rotation = rot,
        .scale = glm::vec3(1,1,1)
    };

    viewportInfo = std::make_unique<ViewportInfo>(vi);
}


__global__ void sendRays(Raytracer::Ray* rays, glm::vec3 forward, int width, int height, int left, int bot, float z){
    int index = threadIdx.x + (blockDim.x * blockIdx.x);
    if(index < width * height){
        int x = index % width;
        int y = index / width;

        rays[index].dir = forward;
        rays[index].origin.x = x + left;
        rays[index].origin.y = y + bot; 
        rays[index].origin.z = z;
    }
}

// send viewport height * width rays into the scene, from the camera to each pixel
// orthographic projection
void Camera::generateRays(){
    float width = viewportInfo->width;
    float height = viewportInfo->height;

    float left = transform.position.x - (width / 2);
    float bot = transform.position.y - (height / 2);

    int size = width * height;

    rays = std::vector<Raytracer::Ray>(size);

    Raytracer::Ray* rawRay = nullptr;
    cudaMallocManaged(&rawRay, size * sizeof(Raytracer::Ray));

    int threads = 256;
    int blocks = (size + threads - 1) / threads;
    sendRays<<<blocks, threads>>>(rawRay,transform.forward(),width,height,left,bot,transform.position.z);

    cudaDeviceSynchronize();

    rays.resize(size);
    std::copy(rawRay, rawRay + size, rays.begin());

    cudaFree(rawRay);

}

float raySphereCollide(Sphere sphere, Raytracer::Ray ray){
    glm::vec3 offset = sphere.position - ray.origin;

    float a = glm::dot(ray.dir, ray.dir);
    float b = -2 * glm::dot(ray.dir, offset);
    float c = glm::dot(offset,offset) - (sphere.radius * sphere.radius);

    float discriminant = b * b - 4 * a * c;
    // missed
    if(discriminant < 0){
        return -1;
    }
    float t = -b - std::sqrt(discriminant) / 2 * a;

    // return t if in fron of the ray, and -1 if the ray is behind the sphere
    return t >= 0.0f ? t : -1;
}

void writeColorsToPPM(std::vector<glm::vec3> colors, float height, float width){
    std::cout << "P3\n" << width << ' ' << height << "\n255\n";

    for(int i = 0; i < colors.size(); i++){
        glm::vec3 color = colors.at(i);

        float ir = color.r * 255.9999f;
        float ig = color.g * 255.9999f; 
        float ib = color.b * 255.9999f;

        std::cout << ir << ' ' << ig << ' ' << ib << '\n';
    }
    std::cout << std::flush;
}

void Camera::shootRays(Sphere sphere){
    glm::vec3 topColor = glm::vec3(0,0,0);
    glm::vec3 botColor = glm::vec3(1,0,0);

    float width = viewportInfo->width;
    float height = viewportInfo->height;

    float bot = transform.position.y - (height / 2);

    float size = width * height;
    std::vector<glm::vec3> colors;
    colors.reserve(size);

    for(int i = 0; i < size; i++){
        Raytracer::Ray ray = rays.at(i);
        float hit = raySphereCollide(sphere, ray);
        // if missed
        if(hit == -1){
            colors.push_back(topColor);
        }
        // hit
        else{
            colors.push_back(botColor);
        }

    }
    writeColorsToPPM(colors, height, width);
}


