#pragma once
#include <cmath>
#include <tuple>
#include <vector>

#include "math/ray.h"
#include "math/vec3.h"
#include "primitives/object.h"
#include "shading/material.h"

class YZRect : public Object {
  public:
    Vec3 center;
    float l;
    float w;

    YZRect(const Material &mat, const Vec3 &center, float l, float w)
        : Object(mat), center(center), l(l), w(w) {}

    Material get_material() override { return mat; }

    hit_result hit(const Ray &ray) override {
        Vec3 ray_origin = ray.get_origin();
        Vec3 ray_direction = ray.get_direction();

        float numerator = center.x - ray_origin.x;
        float denom = ray_direction.x;

        if (denom == 0) return hit_result{0, Vec3(0), Vec3(0), false};

        float t = numerator / denom;
        if (t < 0) return hit_result{0, Vec3(0), Vec3(0), false};

        Vec3 surface_point = ray_origin + ray_direction * t;

        float half_l = l / 2, half_w = w / 2;
        if (surface_point.y < center.y - half_l ||
            surface_point.y > center.y + half_l ||
            surface_point.z < center.z - half_w ||
            surface_point.z > center.z + half_w)
            return hit_result{0, Vec3(0), Vec3(0), false};

        Vec3 normal = (denom > 0) ? Vec3(-1, 0, 0) : Vec3(1, 0, 0);
        return hit_result{t, surface_point, normal, true};
    }
};