#include "../headers/Scenes/SceneLoader.hpp"
#include "../headers/Scenes/BVHTestScene.hpp"

#include "../headers/Hittables/Hittable.cuh"

#include <vector>

Scenes::SceneInfo Scenes::LoadScene(SceneList scene){
    switch (scene){
        case BVHTEST:
            return LoadBVHTestScene();
            break;
        case BUNNYMIRROR:
        // CREATE THIS SCENE
            break;
    }

    
}