#include "../Headers/Scenes/SceneLoader.hpp"
#include "../Headers/Camera/Camera.hpp"
#include "../Headers/Hittables/Mesh.hpp"
#include "../Headers/Util/ObjReader.hpp"
#include <vector>


// chat created
Scenes::SceneInfo LoadBVHTestScene(){
    ViewportInfo vi{
        .near = .001f,
        .far = 20000.0f,
        .width = 960,
        .height = 540
    };

    Camera cam(vi, glm::vec3(0.0f, 60.0f, 160.0f), glm::quat(glm::vec3(glm::radians(-8.0f), glm::radians(90.0f), 0.0f)));

    std::vector<Raytracer::Hittable> shapeList;


    Raytracer::Sphere sphere = Raytracer::Sphere{
        .radius = 50.0f,
        .position = glm::vec3(-20, 0, 0),
    };
// 27, 22, 33
    Raytracer::Hittable sphereHit = Raytracer::Hittable(sphere);
    sphereHit.mat = {
        .albedo = glm::vec3(27.0f / 255.0f, 22.0 / 255.0f, 33 / 255.0f),
        .metallic = 0.0f,
    };

    shapeList.push_back(sphereHit);

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

    shapeList.push_back(mirror1Hit);



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
    shapeList.push_back(mirror2Hit);


    Raytracer::Sphere sun = Raytracer::Sphere{
        .radius = 5000.0f,
        .position = glm::vec3(0.0f, 10000.0f, 0.0f),
    };

    Raytracer::Hittable sunHit = Raytracer::Hittable(sun);
    sunHit.mat = {
        .albedo = glm::vec3(1.0f),
        .emittedColor = glm::vec3(1.0f)
    };

    shapeList.push_back(sunHit);



    Scenes::SceneInfo SI{
        .camera = cam,
        .shapeList = shapeList,
        .viewport = vi
    };

    return SI;

}
