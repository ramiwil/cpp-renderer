#pragma once
#include <cmath>
#include <memory>

#include "math/utils.cpp"
#include "math/vec3.h"
#include "primitives/object.h"
#include "primitives/plane.h"
#include "shading/material.h"

class Light : public Object {
  public:
    Plane pane;
    Vec3 position;
    Vec3 normal;
    float l, w;

    Light(std::shared_ptr<Material> mat, const Vec3 &center,
          const Vec3 &_normal, float _l, float _w)
        : Object(mat), position(center) {
        position = center;
        normal = _normal;
        w = _w;
        l = _l;
        pane = Plane(mat, center, normal, l, w);
    }

    hit_result hit(const Ray &ray) override { return pane.hit(ray); }

    Vec3 get_position() { return position; }

    Vec3 sample() {
        Vec3 ref = std::abs(normal.x) < 0.9f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
        Vec3 tangent = normal.cross(ref).normalize();
        Vec3 bitangent = normal.cross(tangent);

        float x_off = dist(rng) * l / 2.0f;
        float y_off = dist(rng) * w / 2.0f;
        Vec3 point = position + tangent * x_off + bitangent * y_off;

        return point;
    }

    float pdf_area() const { return 1.0f / (l * w); }
};