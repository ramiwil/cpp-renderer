#pragma once
#include "math/vec3.h"

struct Material {
    Vec3 color{1.0f};
    Vec3 emission{0.0f};
    Vec3 diffuse{0.5f};
    Vec3 specular{0.5f};
    float emission_strength{10.0f};
    float reflectivity{0.0f};
    float shininess{32.0f};
};