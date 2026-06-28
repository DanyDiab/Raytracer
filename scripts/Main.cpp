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
        .width = 200,
        .height = 200
    };
    
    Camera cam(vi,glm::vec3(0,0,-10), glm::quat(glm::vec3(0,0,1)));

    Sphere sphere = Sphere(glm::vec3(20,0,0), 20.0f);
    cam.generateRays();
    cam.shootRays(sphere);


    while(window.updateWindow()){

    }

    window.destroyWindow();
}