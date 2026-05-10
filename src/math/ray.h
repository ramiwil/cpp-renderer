#pragma once
#include "math/vec3.h"

#ifdef __CUDA_ARCH__
#define HD __host__ __device__
#else
#define HD
#endif

class Ray {
public:
  Vec3 origin;
  Vec3 direction;

  HD Ray(const Vec3 &origin, const Vec3 &direction)
      : origin(origin), direction(direction) {}

  HD Vec3 at(float t) const { return origin + direction * t; }

  HD Vec3 get_origin() const { return origin; };

  HD Vec3 get_direction() const { return direction; };

  HD Vec3 get_negative_direction() const {
    return Vec3(-direction.x, -direction.y, -direction.z);
  };

  HD Vec3 operator-() const {
    return -direction; // returns opposite of direction
  }
};