#include "./headers/Window.hpp"
#include "headers/Camera.hpp"
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
    
    Camera cam(vi);
    cam.generateRays();
    cam.shootRays();
    while(window.updateWindow()){

    }

    window.destroyWindow();
}