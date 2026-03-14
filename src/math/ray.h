#pragma once
#include "math/vec3.h"

class Ray {
  public:
    Vec3 origin;
    Vec3 direction;

    Ray(const Vec3 &origin, const Vec3 &direction)
        : origin(origin), direction(direction) {}

    Vec3 at(float t) const { return origin + direction * t; }

    Vec3 get_origin() const { return origin; };

    Vec3 get_direction() const { return direction; };

    Vec3 get_negative_direction() const {
        return Vec3(-direction.x, -direction.y, -direction.z);
    };

    Vec3 operator-() const {
        return -direction;  // returns opposite of direction
    }
};