#include "./headers/Window.hpp"

#include "headers/Camera.hpp"
#include "headers/Hittable.cuh"
#include "headers/Ray.cuh"
#include "headers/Sphere.cuh"
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/quaternion.hpp>
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
        .far = 10000.0f,
        .width = 960,
        .height = 540
    };
    
Camera cam(vi, glm::vec3(0.0f, -20.0f, -1000.0f), glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));

Raytracer::Sphere sphere = Raytracer::Sphere{
    .radius = 10000.0f,
    .position = glm::vec3(0.0f, 10200.0f, 0.0f),
};

Raytracer::Sphere sphere1 = Raytracer::Sphere{
    .radius = 100.0f, 
    .position = glm::vec3(200.0f, 100.0f, 0.0f), 
};

Raytracer::Sphere sphere2 = Raytracer::Sphere{
    .radius = 100.0f, 
    .position = glm::vec3(-200.0f, 100.0f, 0.0f), 
};

Raytracer::Sphere sphere3 = Raytracer::Sphere{
    .radius = 100.0f, 
    .position = glm::vec3(0.0f, 100.0f, 0.0f),
};

Raytracer::Hittable hit = Raytracer::Hittable(sphere);
Raytracer::Hittable hit1 = Raytracer::Hittable(sphere1);
Raytracer::Hittable hit2 = Raytracer::Hittable(sphere2);
Raytracer::Hittable hit3 = Raytracer::Hittable(sphere3);

hit.mat = {
    .albedo = glm::vec3(180.0f / 255.0f, 200.0f / 255.0f, 0.0f / 255.0f),
    .metallic = 0.0f
};

hit1.mat = {
    .albedo = glm::vec3(230.0f / 255.0f, 195.0f / 255.0f, 65.0f / 255.0f),
    .metallic = 1.0f
};

hit2.mat = {
    .albedo = glm::vec3(0.95f, 0.95f, 0.95f),
    .metallic = 1.0f
};

hit3.mat = {
    .albedo = glm::vec3(0.15f, 0.35f, 0.75f),
    .metallic = 0.0f
};

    std::vector<std::shared_ptr<Raytracer::Hittable>> shapeList;

    shapeList.push_back(std::make_shared<Raytracer::Hittable>(hit));
    shapeList.push_back(std::make_shared<Raytracer::Hittable>(hit1));
    shapeList.push_back(std::make_shared<Raytracer::Hittable>(hit2));
    shapeList.push_back(std::make_shared<Raytracer::Hittable>(hit3));





    cam.Render(shapeList);

    std::cout << "RENDERING IS DONE! \n INSHALLAH ITS RIGHT!\n";


    while(window.updateWindow()){

    }

    window.destroyWindow();
}