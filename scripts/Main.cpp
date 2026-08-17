#include "headers/Hittables/Triangle.cuh"
#include "headers/Util/PpmWriter.hpp"
#include "headers/Util/Transform.hpp"
#include "headers/Util/Window.hpp"
#include "headers/Hittables/Hittable.cuh"
#include "headers/Camera/Camera.hpp"
#include "headers/RayHits/Ray.cuh"
#include "headers/Hittables/Sphere.cuh"
#include "headers/Hittables/Mesh.hpp"
#include "headers/Camera/RenderFlags.hpp"

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

    Camera cam(vi, glm::vec3(0.0f, 60.0f, 160.0f), glm::quat(glm::vec3(glm::radians(-8.0f), glm::radians(90.0f), 0.0f)));

    
    Raytracer::Sphere ground = Raytracer::Sphere{
        .radius = 10000.0f,
        .position = glm::vec3(0.0f, -10010.0f, 0.0f),
    };

    // 16, 33, 17
    Raytracer::Hittable groundHit = Raytracer::Hittable(ground);
    groundHit.mat = {
        .albedo = glm::vec3(16.0f/255.0f, 33.0f/255.0f, 17.0f/255.0f), 
    };


    Raytracer::Sphere sphere = Raytracer::Sphere{
        .radius = 50.0f,
        .position = glm::vec3(-110.0f, 100.0f, 0.0f),
    };

// 27, 22, 33
    Raytracer::Hittable sphereHit = Raytracer::Hittable(sphere);
    sphereHit.mat = {
        .albedo = glm::vec3(27.0f / 255.0f, 22.0 / 255.0f, 33 / 255.0f), 
        .metallic = 0.0f,
        .roughness = 0.8f
    };

    Raytracer::Triangle mirror1 = Raytracer::Triangle{
        .p1 = glm::vec3(-100.0f, 20.0f, 300.0f),
        .p2 = glm::vec3(-90.0f, 20.0f, -300.0f),
        .p3 = glm::vec3(-95.0f, 400.0f, 0.0f),
    };

// 27, 22, 33
    Raytracer::Hittable mirror1Hit = Raytracer::Hittable(mirror1);
    mirror1Hit.mat = {
        .albedo = glm::vec3(.8f,.8f,.8f), 
        .metallic = 1.0f
    };

    
    Raytracer::Triangle mirror2 = Raytracer::Triangle{
        .p1 = glm::vec3(150.0f, 20.0f, 300.0f),
        .p2 = glm::vec3(140.0f, 20.0f, -300.0f),
        .p3 = glm::vec3(145.0f, 400.0f, 0.0f),
    };

// 27, 22, 33
    Raytracer::Hittable mirror2Hit = Raytracer::Hittable(mirror2);
    mirror2Hit.mat = {
        .albedo = glm::vec3(.8f,.8f,.8f), 
        .metallic = 1.0f
    };

    Raytracer::Sphere sun = Raytracer::Sphere{
        .radius = 5000.0f, 
        .position = glm::vec3(0.0f, 10000.0f, 0.0f),
    };

    Raytracer::Hittable sunHit = Raytracer::Hittable(sun);
    sunHit.mat = {
        .albedo = glm::vec3(1.0f),
        .emittedColor = glm::vec3(1.0f)
    };

    std::vector<Raytracer::Hittable> shapeList;

    shapeList.push_back(groundHit);
    shapeList.push_back(sunHit);
    shapeList.push_back(mirror1Hit);
    shapeList.push_back(mirror2Hit);


    std::vector<Raytracer::Triangle> meshTris = FileOps::ReadInObj("./models/bunny.obj");

    Mesh::Mesh mesh;
    mesh.triangles = meshTris;

    Raytracer::Transform meshTrans;

    meshTrans.position = glm::vec3(-20, 0, 0);
    meshTrans.rotation = glm::vec3(glm::vec3(0, glm::radians(150.0f), 0));
    meshTrans.scale = glm::vec3(55.0f);

    mesh.transform = meshTrans;

    mesh.mat = {
        .albedo = glm::vec3(1.0f,0.0,1.0f),
    };

    Mesh::addMeshToScene(mesh, &shapeList);


    int renderingFlags = Flags::ProcessFlags(argc, argv);

    std::vector<glm::vec3> colors = cam.Render(shapeList, renderingFlags);

    FileOps::writeColorsToPPM(colors, vi.height, vi.width, "./img.ppm");

    while(window.updateWindow()){
        break;
    }

    window.destroyWindow();
}



