#pragma once
#include "scene/camera.h"
#include "scene/scene.h"

struct plane_g {
	int material_id;
	float cx, cy, cz;
	float nx, ny, nz;
	float l, w;
};

struct sphere_g {
	int material_id;
	float cx, cy, cz;
	float radius;
};

struct light_g {
	int material_id;
	float cx, cy, cz;
	float nx, ny, nz;
	float l, w;
};

enum MaterialType { LAMBERTIAN, METAL, GLASS };
struct material_g {
	MaterialType type;
	Vec3 albedo;
	Vec3 emission;
	float emission_strength;
	float ior = 1.52f;
};

struct scene_params {
	int width = 1024;
	int height = 1024;
	int samples = 1;
	int depth = 4;
	light_g *lights;
	int num_lights;
	material_g *materials;
	int num_materials;
	plane_g *planes;
	int num_planes;
	sphere_g *spheres;
	int num_spheres;
};

float *render_gpu(const Camera &cam, scene_params sp);

void render_gpu_interop(float *d_output, float *d_acc, int num_frames, void *d_states,
						const Camera &cam, scene_params sp);

void *dstate_rng_init(int blockSize, int width, int height);

void *allocate_d_mem(void *s_h, int size);

void free_d_mem(void *p_d);
