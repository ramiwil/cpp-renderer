#pragma once
#include "scene/scene.h"
#include "scene/camera.h"


struct scene_params {
    int width = 512;
    int height = 512;
    int samples = 128;
    int depth = 3;
};

struct plane_g {
    int material_id;
    float cx, cy, cz;
    float nx, ny, nz;
    float l, w;
};


enum MaterialType {LAMBERTIAN};

struct material_g {
    MaterialType type;
    Vec3 albedo;
    Vec3 emission;
    float emission_strength;
};

float* render_gpu(const Camera& cam, const Scene& scene, scene_params sp);