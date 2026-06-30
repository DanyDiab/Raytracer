#pragma once

#include "Ray.hpp"
#include "Material.hpp"
#include "Transform.hpp"
namespace Raytracer{
    class Hittable{
        public:
            Hittable() = default;
            virtual ~Hittable();

            virtual float rayCollide(const Raytracer::Ray ray) const = 0;
            Raytracer::Transform transform;
            Material mat;
    };
}