#pragma once
#include <memory>

#include "math/vec3.h"
#include "primitives/object.h"
#include "shading/material.h"
// class Light {
//   public:
//     Light(Vec3 pos, float intensity) : position(pos), intensity(intensity) {}
//     Vec3 get_position() { return position; }
//     float get_intensity() { return intensity; }

//   private:
//     Vec3 position;
//     float intensity;
// };

class Light : public Object {
  public:
    Vec3 center;
    float l, w;

    Light(std::shared_ptr<Material> mat, const Vec3 &center, float l, float w)
        : Object(mat), center(center), l(l), w(w) {}
};