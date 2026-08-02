#include "./headers/Window.hpp"

#include "headers/Camera.hpp"
#include "headers/Hittable.cuh"
#include "headers/Ray.cuh"
#include "headers/Sphere.cuh"
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>
#include <system_error>
#include <vector>

int main(int argc, char** argv){
    Window window;
    if(!window.createWindow()){
        std::cerr << "Something went wrong while making the window\n";
    }
    ViewportInfo vi{
        .near = .01f,
        .far = 20000.0f,
        .width = 960,
        .height = 540
    };
    
Camera cam(vi, glm::vec3(0.0f, 60.0f, -150.0f), glm::quat(glm::vec3(glm::radians(15.0f), 0.0f, 0.0f)));

Raytracer::Sphere ground = Raytracer::Sphere{
    .radius = 10000.0f,
    .position = glm::vec3(0.0f, -10010.0f, 0.0f),
};

Raytracer::Hittable groundHit = Raytracer::Hittable(ground);
groundHit.mat = {
    .albedo = glm::vec3(0.05f, 0.4f, 0.1f), 
    .metallic = 0.0f,
    .roughness = 0.8f
};

Raytracer::Sphere sun = Raytracer::Sphere{
    .radius = 5000.0f, 
    .position = glm::vec3(0.0f, 4000.0f, -5000.0f),
};

Raytracer::Hittable sunHit = Raytracer::Hittable(sun);
sunHit.mat = {
    .albedo = glm::vec3(0.0f),
    .emittedColor = glm::vec3(1.0f)
};


Raytracer::Sphere glassBall = Raytracer::Sphere{
    .radius = 20.0f, 
    .position = glm::vec3(15.0f, 30.0f, -40.0f),
};

Raytracer::Hittable glassHit = Raytracer::Hittable(glassBall);

glassHit.mat = {
    .albedo = glm::vec3(1.0f),
    .transmission = 1.0f,
    .IOR = 1.5,
};

Raytracer::Sphere behindBall = Raytracer::Sphere{
    .radius = 30.0f, 
    .position = glm::vec3(0.0f, 30.0f, 20.0f),
};

Raytracer::Hittable behindHit = Raytracer::Hittable(behindBall);

behindHit.mat = {
    .albedo = glm::vec3(0.0, 0.0f, 1.0f),
};


std::vector<std::shared_ptr<Raytracer::Hittable>> shapeList;

shapeList.push_back(std::make_shared<Raytracer::Hittable>(groundHit));
shapeList.push_back(std::make_shared<Raytracer::Hittable>(sunHit));
shapeList.push_back(std::make_shared<Raytracer::Hittable>(behindHit));
shapeList.push_back(std::make_shared<Raytracer::Hittable>(glassHit));



    cam.Render(shapeList);

    while(window.updateWindow()){
        break;
    }

    window.destroyWindow();
}



