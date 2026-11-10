#include "../Headers/Scenes/SceneLoader.hpp"
#include "../Headers/Scenes/BVHTestScene.hpp"
#include "../Headers/Scenes/BunnyMirrorScene.hpp"


#include "../Headers/Hittables/Hittable.cuh"

#include <vector>

Scenes::SceneInfo Scenes::LoadScene(SceneList scene){
    switch (scene){
        case BVHTEST:
            return LoadBVHTestScene();
            break;
        case BUNNYMIRROR:
            return LoadBunnyMirrorScene();
            break;
    }


}
