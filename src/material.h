#pragma once
#include "vec3.h"

struct Material {
    Vec3 color{255.0f};
    Vec3 diffuse{0.5f};
    Vec3 specular{0.5f};
    float reflectivity{0.0f};
    float shininess{32.0f};
};