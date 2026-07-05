#include <cuda_runtime.h>


#include "headers/Hittable.cuh"
#include "headers/Sphere.cuh"

namespace Raytracer {


    __host__ __device__ Hittable::Hittable(Sphere sphere){
        shapeType = SHAPE_SPHERE;
        this->sphere = sphere;
    }


}