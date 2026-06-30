#pragma once

#include "Ray.hpp"
#include "Material.hpp"
#include "Transform.hpp"
namespace Raytracer{
    class Hittable{
        public:
            Hittable();
            virtual ~Hittable();

            virtual float rayCollide(const Raytracer::Ray ray) const;
            Raytracer::Transform transform;
            Material mat;
    };
}