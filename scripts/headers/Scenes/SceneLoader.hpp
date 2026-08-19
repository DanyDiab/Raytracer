#pragma once

#include "../Hittables/Hittable.cuh"
#include "../Camera/Camera.hpp"

#include <vector>


namespace Scenes{

    enum SceneList{
        BVHTEST,
        BUNNYMIRROR
    };

    struct SceneInfo{
        Camera camera;
        std::vector<Raytracer::Hittable> shapeList;
        ViewportInfo viewport;
    };

    SceneInfo LoadScene(SceneList scene);

}