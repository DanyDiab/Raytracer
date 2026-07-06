#include "./headers/Window.hpp"

#include "headers/Camera.hpp"
#include "headers/Hittable.cuh"
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
        .far = 100.0f,
        .width = 960,
        .height = 540
    };
    
    Camera cam(vi,glm::vec3(0,0,-50), glm::quat(glm::vec3(0,0,0)));

    // Sphere sphere(glm::vec3(0,0,100),glm::quat(glm::vec3(0,180,0)), glm::vec3(50,50,50));
    // sphere.mat.color = glm::vec3(1,0,0);


    // Sphere sphere1(glm::vec3(100,0,70),glm::quat(glm::vec3(0,180,0)), glm::vec3(80,80,80));
    // sphere1.mat.color = glm::vec3(0,1,0);
    Raytracer::Sphere sphere = Raytracer::Sphere{
        .radius = 50.0f,
        .position = glm::vec3(0,0,100),
    };

    Raytracer::Sphere sphere1 = Raytracer::Sphere{
        .radius = 80.0f, 
        .position = glm::vec3(100,0,70), 
    };

    Raytracer::Sphere sphere2 = Raytracer::Sphere{
        .radius = 300.0f, 
        .position = glm::vec3(-450,0,500), 
    };



    Raytracer::Hittable hit = Raytracer::Hittable(sphere);
    Raytracer::Hittable hit1 = Raytracer::Hittable(sphere1);
    Raytracer::Hittable hit2 = Raytracer::Hittable(sphere2);

    
    hit.mat = {
        .color = glm::vec3(1,0,0)
    };

    hit1.mat = {
        .color = glm::vec3(0,1,0)
    };
// 219, 136, 136
    hit2.mat = {
        .color = glm::vec3(219.0/255.0,136.0/255.0,136.0/255.0)
    };
    

    std::vector<std::shared_ptr<Raytracer::Hittable>> shapeList;

    shapeList.push_back(std::make_shared<Raytracer::Hittable>(hit));
    shapeList.push_back(std::make_shared<Raytracer::Hittable>(hit1));
    shapeList.push_back(std::make_shared<Raytracer::Hittable>(hit2));



    cam.generateRays();

    cam.shootRays(shapeList);


    while(window.updateWindow()){

    }

    window.destroyWindow();
}