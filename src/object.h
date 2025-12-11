#pragma once
#include <vector>
#include <tuple>
#include "vec3.h"
#include "material.h"
#include "ray.h"

struct hit_result {
    float t;
    Vec3 point;
    Vec3 normal;
    bool hit{false};
};

class Object {
    public:
        Material mat;
        virtual Material get_material() = 0;
        virtual hit_result hit(const Ray& ray) = 0;
        virtual ~Object() = default;

};