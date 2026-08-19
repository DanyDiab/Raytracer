#include "headers/Util/PpmWriter.hpp"
#include "headers/Util/Window.hpp"
#include "headers/Camera/RenderFlags.hpp"

#include "headers/Scenes/SceneLoader.hpp"

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

    Scenes::SceneList sceneToLoad = Scenes::SceneList::BVHTEST;

    Scenes::SceneInfo sceneInfo = Scenes::LoadScene(sceneToLoad);

    int renderingFlags = Flags::ProcessFlags(argc, argv);

    std::vector<glm::vec3> colors = sceneInfo.camera.Render(sceneInfo.shapeList, renderingFlags);

    FileOps::writeColorsToPPM(colors, sceneInfo.viewport.height, sceneInfo.viewport.width, "./img.ppm");

    while(window.updateWindow()){
        break;
    }

    window.destroyWindow();
}



