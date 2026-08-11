

#include <vector>
#include "../headers/Hittables/Triangle.cuh"
#include "../headers/Hittables/Hittable.cuh"
#include "../headers/Util/Transform.hpp"
#include "../headers/Hittables/Mesh.hpp"

// this will transform the points according to the mesh struct info, then add to the list
void Mesh::addMeshToScene(Mesh mesh, std::vector<Raytracer::Hittable>* hittables){
    Raytracer::Transform trans = mesh.transform;

    glm::mat4 modelMatrix = trans.GetModelMatrix();
    for(Raytracer::Triangle tri : mesh.triangles){
        glm::vec4 wP1 = modelMatrix * glm::vec4(tri.p1, 1);
        glm::vec4 wP2 = modelMatrix * glm::vec4(tri.p2, 1);
        glm::vec4 wP3 = modelMatrix * glm::vec4(tri.p3, 1);

        tri.p1 = wP1;
        tri.p2 = wP2;
        tri.p3 = wP3;

        Raytracer::Hittable hit = Raytracer::Hittable(tri);

        hit.mat = mesh.mat;
        hittables->push_back(hit);
    }
}

