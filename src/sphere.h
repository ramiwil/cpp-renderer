#pragma once
#include <vector>
#include <tuple>
#include "vec3.h"

class sphere {
    public:
        vec3 center;
        float radius;

        sphere(const vec3& center, float radius) : center(center), radius(radius) {}
        std::tuple<int, vec3, vec3> intersect(const vec3& ray_origin, const vec3& ray_direction) {
            vec3 l = center - ray_origin;
            int tca = l.dot(ray_direction);
            if (tca < 0) {
                return std::make_tuple(0, vec3(0,0,0), vec3(0,0,0));
            }

            float determinant = std::sqrt(l.dot(l) - (tca*tca));
            float r_squared = this->radius*this->radius;
            float d_squared = determinant * determinant;
            float thc = std::sqrt(r_squared - d_squared);
            float t0 = tca - thc;
            float t1 = tca + thc;

            vec3 surface_point = ray_origin + ray_direction * t0;
            vec3 surface_normal = (surface_point - center) / radius;

            return std::make_tuple(t0, surface_point, surface_normal);
        }
};
