#pragma once
#include <vector>
#include <tuple>
#include "vec3.h"
#include "material.h"

struct hit_result {
    float t;
    vec3 point;
    vec3 normal;
    bool hit{false};
};

class object {
    public:
        material mat;
        virtual hit_result hit(const vec3 ray_origin, const vec3 ray_direction);
};