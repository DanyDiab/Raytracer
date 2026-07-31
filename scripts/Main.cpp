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

// The pool table surface
Raytracer::Sphere ground = Raytracer::Sphere{
    .radius = 10000.0f,
    .position = glm::vec3(0.0f, -10010.0f, 0.0f), // Surface peaks exactly at y = -10.0f
};

Raytracer::Hittable groundHit = Raytracer::Hittable(ground);
groundHit.mat = {
    .albedo = glm::vec3(0.05f, 0.4f, 0.1f), // Billiard green
    .metallic = 0.0f,
    .roughness = 0.8f
};

// Sun/Light source to illuminate the scene
Raytracer::Sphere sun = Raytracer::Sphere{
    .radius = 5000.0f, 
    .position = glm::vec3(0.0f, 4000.0f, -5000.0f),
};

Raytracer::Hittable sunHit = Raytracer::Hittable(sun);
sunHit.mat = {
    .albedo = glm::vec3(0.0f),
    .emittedColor = glm::vec3(1.0f)
};

std::vector<std::shared_ptr<Raytracer::Hittable>> shapeList;
shapeList.push_back(std::make_shared<Raytracer::Hittable>(groundHit));
shapeList.push_back(std::make_shared<Raytracer::Hittable>(sunHit));

// Standard 15 pool ball colors
glm::vec3 ballColors[15] = {
    glm::vec3(1.0f, 0.8f, 0.1f),    // 1 Yellow
    glm::vec3(0.1f, 0.2f, 0.8f),    // 2 Blue
    glm::vec3(0.8f, 0.1f, 0.1f),    // 3 Red
    glm::vec3(0.4f, 0.1f, 0.6f),    // 4 Purple
    glm::vec3(1.0f, 0.4f, 0.1f),    // 5 Orange
    glm::vec3(0.1f, 0.6f, 0.2f),    // 6 Green
    glm::vec3(0.5f, 0.1f, 0.2f),    // 7 Maroon
    glm::vec3(0.05f, 0.05f, 0.05f), // 8 Black
    glm::vec3(1.0f, 0.9f, 0.2f),    // 9 Yellow stripe
    glm::vec3(0.2f, 0.4f, 0.9f),    // 10 Blue stripe
    glm::vec3(0.9f, 0.2f, 0.2f),    // 11 Red stripe
    glm::vec3(0.6f, 0.2f, 0.8f),    // 12 Purple stripe
    glm::vec3(1.0f, 0.6f, 0.2f),    // 13 Orange stripe
    glm::vec3(0.2f, 0.8f, 0.3f),    // 14 Green stripe
    glm::vec3(0.7f, 0.2f, 0.3f)     // 15 Maroon stripe
};

float radius = 10.0f;
float spacing = 20.0f; // Distance between centers (2 * radius)
float zOffset = 17.3205f; // spacing * sqrt(3) / 2 for equilateral packing
int ballIndex = 0;

for (int row = 0; row < 5; ++row) {
    for (int col = 0; col <= row; ++col) {
        float x = (static_cast<float>(col) - (static_cast<float>(row) / 2.0f)) * spacing;
        float z = static_cast<float>(row) * zOffset;

        Raytracer::Sphere ball = Raytracer::Sphere{
            .radius = radius,
            // y = 0.0f places the bottom of the ball at -10.0f, tangent to the ground
            .position = glm::vec3(x, 0.0f, z) 
        };

        Raytracer::Hittable ballHit = Raytracer::Hittable(ball);
        ballHit.mat = {
            .albedo = ballColors[ballIndex],
            .metallic = 0.25f,
            .roughness = 0.1f // Shiny finish for billiard balls
        };

        shapeList.push_back(std::make_shared<Raytracer::Hittable>(ballHit));
        ballIndex++;
    }
}

    cam.Render(shapeList);

    while(window.updateWindow()){
        break;
    }

    window.destroyWindow();
}