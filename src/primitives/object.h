#pragma once
#include <memory>

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
    std::shared_ptr<Material> mat;
    virtual hit_result hit(const Ray &ray) = 0;
    Object() = default;
    Object(std::shared_ptr<Material> mat) : mat(mat) {}
    virtual ~Object() = default;
};
