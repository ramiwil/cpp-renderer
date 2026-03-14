#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include "vec3.h"

static thread_local std::mt19937 rng(std::random_device{}());
static thread_local std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
static thread_local std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

unsigned char color_to_byte(float c) {
    float gamma_corrected = std::sqrt(std::clamp(c, 0.0f, 1.0f));
    return static_cast<unsigned char>(gamma_corrected * 255.0f);
}


void write_color(std::ostream& out, Vec3& color) {
    out.put(color_to_byte(color.x));
    out.put(color_to_byte(color.y));
    out.put(color_to_byte(color.z));
}

float rand_float() { return dist01(rng); }

Vec3 random_dir(Vec3 normal) {
    Vec3 dir;
    while (true) {
        Vec3 r(dist(rng), dist(rng), dist(rng));
        if (r.length() * r.length() <= 1.0f) {
            dir = r.normalize();
            break;
        }
    }
    if (normal.dot(dir) < 0.0f) {
        return -dir;
    }
    return dir;
}