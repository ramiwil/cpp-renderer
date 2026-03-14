#pragma once
#include <tuple>
#include <vector>

#include "math/ray.h"
#include "math/vec3.h"
#include "shading/material.h"

struct hit_result {
    float t;
    Vec3 point;
    Vec3 normal;
    bool hit{false};
};

class Object {
  public:
    Material mat;
    virtual Material get_material() = 0;
    virtual hit_result hit(const Ray &ray) = 0;
    Object() = default;
    Object(const Material &mat) : mat(mat) {}
    virtual ~Object() = default;
};