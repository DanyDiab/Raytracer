#include "./headers/Window.hpp"
#include "headers/Camera.hpp"
#include "headers/Hittable.hpp"
#include "headers/Sphere.hpp"
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

    Sphere sphere(glm::vec3(0,0,100),glm::quat(glm::vec3(0,180,0)), glm::vec3(50,50,50));
    sphere.mat.color = glm::vec3(1,0,0);


    Sphere sphere1(glm::vec3(100,0,70),glm::quat(glm::vec3(0,180,0)), glm::vec3(80,80,80));
    sphere1.mat.color = glm::vec3(0,1,0);

    std::vector<std::shared_ptr<Raytracer::Hittable>> shapeList;

    shapeList.push_back(std::make_shared<Sphere>(sphere));
    shapeList.push_back(std::make_shared<Sphere>(sphere1));


    cam.generateRays();

    cam.shootRays(shapeList);


    while(window.updateWindow()){

    }

    window.destroyWindow();
}