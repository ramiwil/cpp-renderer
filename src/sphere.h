#pragma once
#include <vector>
#include <tuple>
#include <cmath>
#include "vec3.h"
#include "object.h"
#include "ray.h"
#include "material.h"

class Sphere : public Object {
    public:
        Material mat;
        Vec3 center;
        float radius;

        Sphere(const Material& mat, const Vec3& center, float radius) : mat(mat), center(center), radius(radius) {}
        
        Material get_material() override {
            return mat;
        }

        hit_result hit(const Ray& ray) override {
            Vec3 ray_origin = ray.getOrigin();
            Vec3 ray_direction = ray.getDirection();

            Vec3 l = center - ray_origin;
            float tca = l.dot(ray_direction);
            if (tca < 0.0) {
                return hit_result{0, Vec3(0), Vec3(0), false};
            }

            float d2 = l.dot(l) - tca * tca;
            if (d2 > radius * radius) return hit_result{0, Vec3(0), Vec3(0), false};
            float thc = std::sqrt(radius * radius - d2);
            float t0 = tca - thc;
            float t1 = tca + thc;

            float t = (t0 > 0) ? t0 : t1;
            if (t < 0) return hit_result{0, Vec3(0), Vec3(0), false};

            Vec3 surface_point = ray_origin + ray_direction * t;
            Vec3 surface_normal = (surface_point - center).normalize();


            return hit_result{t0, surface_point, surface_normal, true};
        }
};
