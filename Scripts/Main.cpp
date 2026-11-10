#include "Headers/Util/PpmWriter.hpp"
#include "Headers/Util/Window.hpp"
#include "Headers/Camera/RenderFlags.hpp"

#include "Headers/Scenes/SceneLoader.hpp"
#include "Headers/Vulkan/Vulkan.hpp"

#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <memory>
#include <vector>



int main(int argc, char** argv){
    Scenes::SceneList sceneToLoad = Scenes::SceneList::BUNNYMIRROR;

    Scenes::SceneInfo sceneInfo = Scenes::LoadScene(sceneToLoad);

    int renderingFlags = Flags::ProcessFlags(argc, argv);

    // std::vector<glm::vec3> colors = sceneInfo.camera.Render(sceneInfo.shapeList, renderingFlags);

    // FileOps::writeColorsToPPM(colors, sceneInfo.viewport.height, sceneInfo.viewport.width, "./img.ppm");

    Vulkan vk;
    vk.run();
}
