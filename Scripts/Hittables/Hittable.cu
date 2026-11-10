#include <cuda_runtime.h>
#include <cstdio>

#include "../Headers/RayHits/HitRecord.hpp"
#include "../Headers/Hittables/Hittable.cuh"
#include "../Headers/RayHits/Ray.cuh"
#include "../Headers/Hittables/Sphere.cuh"
#include "../Headers/Hittables/Triangle.cuh"


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
                normal = SphereRayNormal(shape.sphere, ray, distance);

                break;
            }
            case(SHAPE_TRIANGLE):{
                distance = TriangleRayCollide(shape.triangle, ray);
                normal = TriangleNormal(shape.triangle, ray, distance);
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
        hi.normal = normal;

        return hi;
    }



}
