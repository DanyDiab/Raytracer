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
        .far = 100.0f,
        .width = 960,
        .height = 540
    };
    
    Camera cam(vi,glm::vec3(0,0,-50), glm::quat(glm::vec3(0,0,0)));

    Raytracer::Sphere sphere = Raytracer::Sphere{
        .radius = 200.0f,
        .position = glm::vec3(0,0,700),
    };

    Raytracer::Sphere sphere1 = Raytracer::Sphere{
        .radius = 200.0f, 
        .position = glm::vec3(400,0,150), 
    };

    Raytracer::Sphere sphere2 = Raytracer::Sphere{
        .radius = 300.0f, 
        .position = glm::vec3(-450,0,500), 
    };

    Raytracer::Sphere sphere3 = Raytracer::Sphere{
        .radius = 300.0f, 
        .position = glm::vec3(0,300,600), 
    };


    Raytracer::Hittable hit = Raytracer::Hittable(sphere);
    Raytracer::Hittable hit1 = Raytracer::Hittable(sphere1);
    Raytracer::Hittable hit2 = Raytracer::Hittable(sphere2);
    Raytracer::Hittable hit3 = Raytracer::Hittable(sphere3);

    
    hit.mat = {
        .color = glm::vec3(1,1,1)
    };

    hit1.mat = {
        .color = glm::vec3(.1,1,.1)
    };
// 219, 136, 136
    hit2.mat = {
        .color = glm::vec3(219.0/255.0,136.0/255.0,136.0/255.0)
    };
    hit3.mat = {
        .color = glm::vec3(.1,.1,1)
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