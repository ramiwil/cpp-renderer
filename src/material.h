#pragma once
#include "vec3.h"

struct Material {
    Vec3 color{255.0f};
    Vec3 diffuse{255.0f};
    Vec3 specular{255.0f};
    float reflectivity{32.0f};
    float shininess{0.0f};
};