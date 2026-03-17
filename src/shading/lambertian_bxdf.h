#pragma once
#include <cmath>

#include "math/utils.cpp"
#include "shading/bxdf.h"

class LambertianBxDF : public BxDF {
    Vec3 albedo;

  public:
    LambertianBxDF(Vec3 albedo) : albedo(albedo) {}

    BxDFSample sample(Vec3 normal, Vec3 /*dir_in*/) override {
        float x, y;
        while (true) {
            x = dist(rng);
            y = dist(rng);
            if (x * x + y * y <= 1.0f) break;
        }
        float z = std::sqrt(1.0f - x * x - y * y);

        Vec3 ref = std::abs(normal.x) < 0.9f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
        Vec3 tangent = normal.cross(ref).normalize();
        Vec3 bitangent = normal.cross(tangent);

        Vec3 dir = (tangent * x + bitangent * y + normal * z).normalize();
        float pdf = normal.dot(dir) / M_PI;
        return {dir, pdf};
    }

    Vec3 evaluate(Vec3 /*normal*/, Vec3 /*omega_i*/,
                  Vec3 /*omega_o*/) override {
        return albedo / M_PI;
    }
};
