#pragma once
#include "primitives/object.h"

class XZRect : public Object {
  public:
    Vec3 center;
    float l;
    float w;

    XZRect(std::shared_ptr<Material> mat, const Vec3 &center, float l, float w)
        : Object(mat), center(center), l(l), w(w) {}

    hit_result hit(const Ray &ray) override {
        Vec3 ray_origin = ray.get_origin();
        Vec3 ray_direction = ray.get_direction();

        float numerator = center.y - ray_origin.y;
        float denom = ray_direction.y;

        if (denom == 0) return hit_result{0, Vec3(0), Vec3(0), false};

        float t = numerator / denom;
        if (t < 0) return hit_result{0, Vec3(0), Vec3(0), false};

        Vec3 surface_point = ray_origin + ray_direction * t;

        float half_l = l / 2, half_w = w / 2;
        if (surface_point.x < center.x - half_l ||
            surface_point.x > center.x + half_l ||
            surface_point.z < center.z - half_w ||
            surface_point.z > center.z + half_w)
            return hit_result{0, Vec3(0), Vec3(0), false};

        Vec3 normal = (denom > 0) ? Vec3(0, -1, 0) : Vec3(0, 1, 0);
        return hit_result{t, surface_point, normal, true};
    }
};