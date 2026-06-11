#include <omp.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "gpu_render.h"
#include "math/ray.h"
#include "math/utils.cpp"
#include "math/vec3.h"
#include "primitives/plane.h"
#include "primitives/sphere.h"
#include "scene/camera.h"
#include "scene/scene.h"
#include "shading/material.h"
#include "stb_image_write.h"

std::vector<plane_g> generate_cornell_box() {
	std::vector<plane_g> ps;
	float size = 100.0f;
	plane_g ceiling = {0, 0.0f, size / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, size, size};
	ps.emplace_back(ceiling);

	plane_g floor = {0, 0.0f, -size / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, size, size};
	ps.emplace_back(floor);

	plane_g back_wall = {0, 0.0f, 0.0f, size / 2.0f, 0.0f, 0.0f, 1.0f, size, size};
	ps.emplace_back(back_wall);

	plane_g red_wall = {1, size / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, size, size};
	ps.emplace_back(red_wall);

	plane_g green_wall = {2, -size / 2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, size, size};
	ps.emplace_back(green_wall);

	return ps;
}

std::vector<sphere_g> generate_balls() {
	std::vector<sphere_g> ps;
	sphere_g s1 = {6, 0.0f, -35.0f, 0.0f, 15.0f};
	ps.emplace_back(s1);

	sphere_g s2 = {5, 25.0f, -35.0f, -25.0f, 15.0f};
	ps.emplace_back(s2);

	sphere_g s3 = {4, -25.0f, -35.0f, 25.0f, 15.0f};
	ps.emplace_back(s3);

	return ps;
}

std::vector<material_g> generate_materials() {
	std::vector<material_g> mats;
	material_g white = {LAMBERTIAN, Vec3(1.0f), Vec3(0.0f), 0.0f};
	mats.emplace_back(white);
	material_g red = {LAMBERTIAN, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f), 0.0f};
	mats.emplace_back(red);
	material_g green = {LAMBERTIAN, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f), 0.0f};
	mats.emplace_back(green);
	material_g light = {LAMBERTIAN, Vec3(1.0f), Vec3(1.0f, 1.0f, 1.0f), 3.0f};
	mats.emplace_back(light);
	material_g metal = {METAL, Vec3(1.0f), Vec3(1.0f, 0.0f, 1.0f), 0.0f};
	mats.emplace_back(metal);
	material_g glass = {GLASS, Vec3(1.0f, 1.0f, 1.0f), Vec3(0.0f), 0.0f};
	mats.emplace_back(glass);
	material_g yellow = {LAMBERTIAN, Vec3(1.0f, 1.0f, 0.0f), Vec3(0.0f), 0.0f};
	mats.emplace_back(yellow);

	return mats;
}

std::vector<light_g> generate_lights() {
	std::vector<light_g> lights;
	light_g l1 = {3, 0.0f, 49.9f, 0.0f, 0.0f, 1.0f, 0.0f, 30.0f, 30.0f};
	lights.emplace_back(l1);

	return lights;
}

int main() {
	scene_params sp;

	sp.samples = 512;

	auto materials = generate_materials();
	sp.materials = materials.data();
	sp.num_materials = materials.size();

	auto planes = generate_cornell_box();
	sp.planes = planes.data();
	sp.num_planes = planes.size();

	auto spheres = generate_balls();
	sp.spheres = spheres.data();
	sp.num_spheres = spheres.size();

	auto lights = generate_lights();
	sp.lights = lights.data();
	sp.num_lights = lights.size();

	Camera cam(Vec3(0.0f, 0.0f, -190.0), // position
			   Vec3(0.0f, 0.0f, 0.0f),	 // target
			   Vec3(0.0f, 1.0f, 0.0f),	 // up
			   39.3f,					 // fov
			   sp.width, sp.height);

	float *frame_buffer = render_gpu(cam, sp);

	// write to png
	std::vector<uint8_t> out(sp.width * sp.height * 3);
	for (int i = 0; i < sp.width * sp.height * 3; i++) {
		out[i] = float_to_color(frame_buffer[i]);
	}

	stbi_write_png("renders/output_gpu.png", sp.width, sp.height, 3, out.data(), sp.width * 3);

	return 0;
}
