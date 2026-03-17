#pragma once
#include "math/vec3.h"
#include "shading/bxdf.h"
#include "shading/lambertian_bxdf.h"

class Material {
  public:
    Vec3 emission{0.0f};
    float emission_strength{0.0f};
    virtual BxDF *get_bxdf() = 0;
    virtual ~Material() = default;
};

class LambertianMaterial : public Material {
    LambertianBxDF bxdf;

  public:
    LambertianMaterial(Vec3 albedo) : bxdf(albedo) {}
    BxDF *get_bxdf() override { return &bxdf; }
};
