#pragma once
#include <vector>
#include <tuple>
#include <cmath>
#include "vec3.h"
#include "object.h"
#include "ray.h"
#include "material.h"

class YXRect : public Object {
    public:
        Material mat;
        Vec3 center;
        float l;
        float w;

        YXRect(const Material& mat, const Vec3& center, float l, float w): mat(mat), center(center), l(l), w(w) {}

        Material get_material() override {
            return mat;
        }

        hit_result hit(const Ray& ray) override {
            float denom = ray.get_direction().z;
            if (denom == 0) return {0, {}, {}, false};
            float t = (center.z - ray.get_origin().z) / denom;
            if (t < 0) return {0, {}, {}, false};
            Vec3 p = ray.get_origin() + ray.get_direction() * t;
            if (p.x < center.x - w/2 || p.x > center.x + w/2 ||
                p.y < center.y - l/2 || p.y > center.y + l/2)
                return {0, {}, {}, false};
            Vec3 normal = denom > 0 ? Vec3(0,0,-1) : Vec3(0,0,1);
        return {t, p, normal, true};
    }
};