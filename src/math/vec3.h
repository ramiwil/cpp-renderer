#pragma once
#include <cmath>

#ifdef __CUDA_ARCH__
  #define HD __host__ __device__
#else
  #define HD
#endif

class Vec3 {
  public:
    float x, y, z;
    HD Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    HD Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    HD Vec3(float value) : x(value), y(value), z(value) {}

    HD float dot(const Vec3 &other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    HD Vec3 cross(const Vec3 &other) const {
        return Vec3(y * other.z - z * other.y, z * other.x - x * other.z,
                    x * other.y - y * other.x);
    }

    HD Vec3 normalize() const {
        float length = std::sqrt(x * x + y * y + z * z);
        return Vec3(x / length, y / length, z / length);
    }

    HD float length() const { return std::sqrt(x * x + y * y + z * z); }

    HD Vec3 operator+(const Vec3 &other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    HD void operator+=(const Vec3 &other) {
        x = x + other.x;
        y = y + other.y;
        z = z + other.z;
    }

    HD void operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
    }

    HD Vec3 operator-(const Vec3 &other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    HD Vec3 operator*(const Vec3 &other) const {
        return Vec3(x * other.x, y * other.y, z * other.z);
    }

    HD Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    HD Vec3 operator/(float scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    HD Vec3 operator+(float scalar) const {
        return Vec3(x + scalar, y + scalar, z + scalar);
    }

    HD Vec3 operator-() const {
        return Vec3(-x, -y, -z);  // returns opposite of direction
    }
};

inline Vec3 operator*(float scalar, const Vec3 &v) { return v * scalar; }