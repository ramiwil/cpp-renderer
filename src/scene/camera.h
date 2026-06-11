#pragma once
#include <cmath>

#include "math/ray.h"
#include "math/vec3.h"

#ifdef __CUDA_ARCH__
#define HD __host__ __device__
#else
#define HD
#endif

class Camera {
  public:
	Vec3 position;
	Vec3 target;
	Vec3 world_up;
	Vec3 camera_up;
	Vec3 forward;
	Vec3 right;

	float fov;
	float aspect_ratio;

	HD Camera(const Vec3 &position, const Vec3 &target, const Vec3 &world_up, float fov, int width,
			  int height)
		: position(position), target(target), world_up(world_up), camera_up(0.0f), forward(0.0f),
		  right(0.0f), fov(fov), aspect_ratio(float(width) / float(height)) {
		updateBases();
	};

	HD Ray generate_ray(float x, float y) const {
		float theta = fov * M_PI / 180.0f;
		float half_height = std::tan(theta / 2.0f);
		float half_width = aspect_ratio * half_height;

		float u = (2 * x - 1) * half_width;
		float v = (1 - 2 * y) * half_height;

		Vec3 ray_direction = (forward + (right * u) + (camera_up * v)).normalize();
		return Ray(position, ray_direction);
	};

	HD void updateBases() {
		forward = (target - position).normalize();
		right = forward.cross(world_up).normalize();
		camera_up = right.cross(forward).normalize();
	}
};
