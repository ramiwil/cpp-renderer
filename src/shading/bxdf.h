#pragma once
#include "math/vec3.h"

struct BxDFSample {
    Vec3 dir;
    float pdf;
};

class BxDF {
  public:
    virtual BxDFSample sample(Vec3 normal, Vec3 dir_in) = 0;
    virtual Vec3 evaluate(Vec3 normal, Vec3 dir_in, Vec3 dir_out) = 0;
    virtual ~BxDF() = default;
};
