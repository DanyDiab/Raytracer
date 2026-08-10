#include "headers/Hittables/Triangle.cuh"
#include "headers/Util/Window.hpp"

#include "headers/Hittables/Hittable.cuh"
#include "headers/Camera/Camera.hpp"
#include "headers/RayHits/Ray.cuh"
#include "headers/Hittables/Sphere.cuh"
#include "headers/Util/ObjReader.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>
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
    
        Camera cam(vi, glm::vec3(0.0f, 60.0f, -250.0f), glm::quat(glm::vec3(glm::radians(-8.0f), 0.0f, 0.0f)));

    
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

    std::vector<Raytracer::Triangle> mesh = ReadInObj("./models/bunny.obj");


    std::vector<std::shared_ptr<Raytracer::Hittable>> shapeList;

    float scale = 1000.0f;

    for(Raytracer::Triangle tri : mesh){

        tri.p1 *= scale;
        tri.p2 *= scale;
        tri.p3 *= scale;

        Raytracer::Hittable triHit = Raytracer::Hittable(tri);

        triHit.mat = {
            .albedo = glm::vec3(1.0f,0.0f,1.0f)
        };

        shapeList.push_back(std::make_shared<Raytracer::Hittable>(triHit));
    }

    shapeList.push_back(std::make_shared<Raytracer::Hittable>(groundHit));
    shapeList.push_back(std::make_shared<Raytracer::Hittable>(sunHit));

    cam.Render(shapeList);

    while(window.updateWindow()){
        break;
    }

    window.destroyWindow();
}



