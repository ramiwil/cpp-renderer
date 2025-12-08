#pragma once
#include "vec3.h"

struct material {
    vec3 color{255.0f};
    vec3 diffuse{255.0f};
    vec3 specular{255.0f};
    float reflectivity{32.0f};
    float shininess{0.0f};
};