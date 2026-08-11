#pragma once


#include <vector>
#include "../Hittables/Triangle.cuh"
#include "../Hittables/Hittable.cuh"
#include "../Util/Transform.hpp"
#include "Material.hpp"

namespace Mesh{
    struct Mesh{
        std::vector<Raytracer::Triangle> triangles;
        Raytracer::Transform transform;
        Raytracer::Material mat;
    };
    
    
    void addMeshToScene(Mesh mesh, std::vector<Raytracer::Hittable>* hittables);
}
