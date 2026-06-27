#include "headers/Camera.hpp"
#include "headers/Transform.hpp"
#include <glm/common.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

#include "headers/Ray.hpp"


Camera::Camera(ViewportInfo vi) {
    transform = {
        .position = glm::vec3(0,0,0),
        .rotation = glm::quat(1,0,0,0),
        .scale = glm::vec3(1,1,1)
    };

    viewportInfo = std::make_unique<ViewportInfo>(vi);
}

Camera::Camera(ViewportInfo vi, glm::vec3 pos, glm::quat rot){
    transform = {
        .position = pos,
        .rotation = rot,
        .scale = glm::vec3(1,1,1)
    };

    viewportInfo = std::make_unique<ViewportInfo>(vi);
}

// send viewport height * width rays into the scene, from the camera to each pixel
// orthographic projection
void Camera::generateRays(){
    float width = viewportInfo->width;
    float height = viewportInfo->height;

    float left = transform.position.x - (width / 2);
    float bot = transform.position.y - (height / 2);


    rays.reserve(width * height);

    for(int y = 0; y < height; y++){
        for(int x = 0; x < width; x++){
            glm::vec3 pos = glm::vec3(x + left, y + bot, transform.position.z);
            Raytracer::Ray ray(pos,transform.forward());
            rays.push_back(ray);
        }
    }
}

void writeColorsToPPM(std::vector<glm::vec3> colors, float height, float width){
    std::cout << "P3\n" << width << ' ' << height << "\n255\n";

    for(int i = 0; i < colors.size(); i++){
        glm::vec3 color = colors.at(i);

        float ir = color.r * 255.9999f;
        float ig = color.g * 255.9999f; 
        float ib = color.b * 255.9999f;

        std::cout << ir << ' ' << ig << ' ' << ib << '\n';
    }
    std::cout << std::flush;

}

void Camera::shootRays(){
    glm::vec3 topColor = glm::vec3(1,0,0);
    glm::vec3 botColor = glm::vec3(0,0,1);

    float width = viewportInfo->width;
    float height = viewportInfo->height;

    float bot = transform.position.y - (height / 2);

    float size = width * height;
    std::vector<glm::vec3> colors;
    colors.reserve(size);



    for(int i = 0; i < size; i++){
        Raytracer::Ray ray = rays.at(i);
        
        float currY = ray.origin.y;
        
        float t = (currY - bot) / height;

        glm::vec3 blended = glm::mix(botColor,topColor, t);
        colors.push_back(blended);
    }
    writeColorsToPPM(colors, height, width);
}


