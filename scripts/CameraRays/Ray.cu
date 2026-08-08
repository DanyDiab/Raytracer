#include <cstdio>
#include <glm/common.hpp>
#include <glm/exponential.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>

#include "../headers/RayHits/Ray.cuh"
#include "../headers/Hittables/Hittable.cuh"
#include "../headers/RayHits/HitRecord.hpp"
#include "../headers/Camera//CameraRayGenerationInfo.hpp"
#include "../headers/Hittables/Material.hpp"
#include "../headers/Util/PRNG.cuh"


__device__ Raytracer::HitRecord Raytracer::Ray::RayIntersectShapes(Raytracer::Hittable* hittables, const int numHittables){
    Raytracer::HitRecord closestRecord;
    closestRecord.hitDistance = -1.0f;
    closestRecord.normal = glm::vec3(0);
    
    // found closer hit point
    for(int i = 0; i < numHittables; i++){
        Raytracer::Hittable& shape = hittables[i];
        Raytracer::HitRecord rayHR = shape.rayCollide(*this);
        if(rayHR.hitDistance < -.999999f || rayHR.hitDistance < 0.001f) continue;

        // found better hit
        if((closestRecord.hitDistance == -1.0f) || rayHR.hitDistance < closestRecord.hitDistance){

            closestRecord.hitDistance = rayHR.hitDistance;
            closestRecord.normal = rayHR.normal;
            closestRecord.mat = shape.mat;
        }
    }

    return closestRecord;
}

__device__ Raytracer::Ray Raytracer::generateRayWithDeviation(CameraRayGenerationInfo camInfo, double currTime, int index, curandState_t* prngState){

    glm::vec3 deviation = PRNG::randomUnitVec(prngState);

    int rasterX = index % camInfo.width;
    int rasterY = index / camInfo.width;

    Raytracer::Ray ray;

    switch(camInfo.projectionType){
        case ORTHOGRAPHIC:{
            glm::vec3 pixelPos = camInfo.camPos + (camInfo.up * (rasterY + camInfo.botOffset + deviation.y)) + (camInfo.right * (rasterX + camInfo.leftOffset +  deviation.x));

            glm::vec3 origin = pixelPos;

            glm::vec3 dir = camInfo.forward;

            ray.dir = dir;
            ray.origin = origin;
            break;
        }
        case PERSPECTIVE:{
            // add .5 to be in the middle of the pixel
            float NDCx = (static_cast<float>(rasterX) + .5f) / camInfo.width;
            float NDCy = (static_cast<float>(rasterY) + .5f) / camInfo.height;
            
            float screenSpaceX = (NDCx * 2) - 1;
            // flip Y to match screen space
            float screenSpaceY = 1 - (NDCy * 2);
            
            float aspectRatio = static_cast<float>(camInfo.width) / camInfo.height;
            
            float fovRadians = glm::radians(camInfo.fov);
            float fovScaler = tan(fovRadians * .5f);


            float pixelX = screenSpaceX * aspectRatio * fovScaler;
            float pixelY = screenSpaceY * fovScaler;

            glm::vec3 camPlanePos = glm::normalize(glm::vec3(camInfo.forward + (camInfo.right * pixelX) + (camInfo.up * pixelY)));

            glm::vec3 origin = camInfo.camPos;
            glm::vec3 dir = camPlanePos;

            ray.dir = dir;
            ray.origin = origin;
            break;
        }
        default:
            printf("unkown cam type");
            break;

    }

    return ray;
}


__device__ glm::vec3 diffuseScatterDir(glm::vec3 normal, curandState_t* state){
    return  normal + PRNG::randomUnitVecSameHemisphere(normal, state);
}

__device__ glm::vec3 reflect(glm::vec3 incidentAngle, glm::vec3 normal){
    // subtract height twice and translate

    return incidentAngle - (2 * glm::dot(incidentAngle, normal) * normal);
}


__device__ glm::vec3 metallicScatterDir(glm::vec3 incidentAngle, glm::vec3 normal, float roughness, curandState_t* state){
    glm::vec3 reflectDir = reflect(incidentAngle, normal);
    // add rand scaled by roughness
    glm::vec3 fuzz = PRNG::randomUnitVec(state) * roughness;
    glm::vec3 dir = reflectDir + fuzz;

    return dir;
}

__device__ float shlickReflectance(float IOR, float cos){
    
     
    float clampedCos = glm::clamp(cos, 0.0f, 1.0f);
    float r0 = (1 - IOR) / (1 + IOR);

    float r = r0 * r0;

    return r + (1-r) * glm::pow(1 - clampedCos, 5);
}

// IOR = index of refraction
// flag indicaets if refraction was allowed or not
__device__ glm::vec4 dielctricScatterDir(float IOR, glm::vec3 incidentAngle, glm::vec3 normal, curandState_t* state){
        
    float rawCos = glm::dot(incidentAngle, normal);

    glm::vec3 correctedNormal = normal;
    float correctedCos;

    // entering, assuming from AIR
    float etaDiff;
    if(rawCos < 0){
        etaDiff = 1.0f/IOR;
        correctedCos = -rawCos;
    }
    // exiting (once again assuming air)
    else{
        correctedNormal = -normal;
        etaDiff = IOR;
        correctedCos = rawCos;
    }

    float angleOfIncidence = glm::min(glm::dot(-incidentAngle, correctedNormal), 1.0f);
    
    glm::vec3 perp = etaDiff * (incidentAngle + angleOfIncidence * correctedNormal);

    // check total internal reflection
    float k = 1.0f - glm::dot(perp,perp);


    bool shouldReflect = k < 0.0f ? true : shlickReflectance(etaDiff, correctedCos) > PRNG::randFloat(state);
    if(shouldReflect){
        // must reflect
        glm::vec3 newDir = reflect(incidentAngle, correctedNormal);
        return glm::vec4(newDir,-1.0);
    }

    glm::vec3 parallel = -glm::sqrt(glm::abs(k)) * correctedNormal;

    glm::vec3 refractedDir = parallel + perp;

    glm::vec4 refractWithFlag = glm::vec4(refractedDir, 1.0);

    return refractWithFlag;
}

// first 3 components = dir
// last component is a flag that indicates if it was a refraction
// if last component > 0 = refraction happened
// else no refraction
// flag is used to determine which way to nudge the origin of the new ray
__device__ glm::vec4 Raytracer::Ray::determineScatterDirection(Raytracer::HitRecord record, curandState_t* state){
    Material mat = record.mat;

    float metallic = mat.metallic;

    float randT = PRNG::randFloat(state);

    float transmission = mat.transmission;

    if(randT < transmission){
        // dielectric
        float IOR = mat.IOR;
        glm::vec4 refractionWithFlag = dielctricScatterDir(IOR, this->dir, record.normal, state);
        return refractionWithFlag;
    }

    bool metallicScatter = randT < metallic;

    if(metallicScatter){
        float roughness = mat.roughness;
        glm::vec3 dir = metallicScatterDir(this->dir,record.normal,roughness,state);
        return glm::vec4(glm::normalize(dir), -1.0f);
    }
    else{
        glm::vec3 dir = diffuseScatterDir(record.normal, state);
        return glm::vec4(glm::normalize(dir), -1.0f);
    }
}




