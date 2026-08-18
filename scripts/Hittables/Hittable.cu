#include <cuda_runtime.h>
#include <cstdio>

#include "../headers/RayHits/HitRecord.hpp"
#include "../headers/Hittables/Hittable.cuh"
#include "../headers/RayHits/Ray.cuh"
#include "../headers/Hittables/Sphere.cuh"
#include "../headers/Hittables/Triangle.cuh"


namespace Raytracer {
    __host__ __device__ Hittable::Hittable(Sphere sphere){
        shapeType = SHAPE_SPHERE;
        this->shape.sphere = sphere;
    }

    __host__ __device__ Hittable::Hittable(Triangle triangle){
        shapeType = SHAPE_TRIANGLE;
        this->shape.triangle = triangle;
    }

    // __host__ __device__ Hittable::Hittable()

    __device__ Raytracer::HitRecord Hittable::rayCollide(const Raytracer::Ray ray) const{
        // printf("Inside Hittable: %f", sphere.radius);
        Raytracer::HitRecord hi = {
            .hitDistance = -1.0f,
            .normal = glm::vec3(0),
        };

        float distance;
        glm::vec3 normal;
        switch(shapeType){
            case(SHAPE_SPHERE):{
                distance = SphereRayCollide(shape.sphere, ray);

                break;
            }
            case(SHAPE_TRIANGLE):{
                distance = TriangleRayCollide(shape.triangle, ray);
                break;
            }
            default:{
                printf("Shape Type Not Recognized | how did we get here?!!?!?!?!?!?!?!!!????");
                break;
            }
        }

        if(distance == -1.0f){
            return hi;
        }

        hi.hitDistance = distance;

        return hi;
    }

__device__ glm::vec3 Hittable::getShapeNormal(const float distance, const Raytracer::Ray& ray){
    glm::vec3 normal;
    switch(shapeType){
        case(Raytracer::SHAPE_SPHERE):{
            normal = SphereRayNormal(shape.sphere, ray, distance);

            break;
        }
        case(Raytracer::SHAPE_TRIANGLE):{
            normal = TriangleNormal(shape.triangle, ray, distance);
            break;
        }
        default:{
            printf("Shape Type Not Recognized | how did we get here?!!?!?!?!?!?!?!!!????");
            break;
        }
    }

    return normal;
}



}