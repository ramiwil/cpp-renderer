#pragma once
#include "primitives/object.h"

class Plane : public Object {
  public:
    Vec3 center;
    Vec3 normal;
    float l, w;

    Plane(std::shared_ptr<Material> mat, const Vec3 &center, const Vec3 &normal,
          float l, float w)
        : Object(mat), center(center), normal(normal), l(l), w(w) {}

    Plane() : center(Vec3(0.0, 0.0, 0.0)), normal(Vec3(0.0, 1.0, 0.0)), l(1.0), w(1.0) {};

    hit_result hit(const Ray &ray) override {
        Vec3 ray_origin = ray.get_origin();
        Vec3 ray_direction = ray.get_direction();

        float numerator = (center - ray_origin).dot(normal);
        float denom = ray_direction.dot(normal);

        if (denom == 0) return hit_result{0, Vec3(0), Vec3(0), false};

        float t = numerator / denom;
        if (t < 0) return hit_result{0, Vec3(0), Vec3(0), false};

        Vec3 surface_point = ray_origin + ray_direction * t;

        Vec3 ref = std::abs(normal.x) < 0.9f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
        Vec3 tangent = normal.cross(ref).normalize();
        Vec3 bitangent = normal.cross(tangent);

        Vec3 proj_hit = (surface_point - center);
        float proj_tan = proj_hit.dot(tangent);
        float proj_bitan = proj_hit.dot(bitangent);

        if (std::abs(proj_tan) > l / 2 || std::abs(proj_bitan) > w / 2)
            return hit_result{0, Vec3(0), Vec3(0), false};

        Vec3 norm = (denom > 0) ? -normal : normal;

        return hit_result{t, surface_point, norm, true};
    }

    
};
