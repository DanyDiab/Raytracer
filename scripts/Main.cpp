#include "./headers/Window.hpp"
#include "headers/Camera.hpp"
#include "headers/Sphere.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <system_error>

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
    
    Camera cam(vi,glm::vec3(0,0,0), glm::quat(glm::vec3(0,0,0)));

    Sphere sphere(glm::vec3(0,0,100),glm::quat(glm::vec3(0,180,0)), glm::vec3(50,50,50));
    cam.generateRays();
    cam.shootRays(sphere);


    while(window.updateWindow()){

    }

    window.destroyWindow();
}