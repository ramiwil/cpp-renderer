#pragma once
#include <cmath>

class Vec3 {
    public:
        float x, y, z;
        Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
        Vec3(float value) : x(value), y(value), z(value) {}

        const Vec3 operator+(const Vec3& other) const {
            return Vec3(x + other.x, y + other.y, z + other.z);
        }

        const Vec3 operator-(const Vec3& other) const {
            return Vec3(x - other.x, y - other.y, z - other.z);
        }

        const Vec3 operator*(float scalar) const {
            return Vec3(x * scalar, y * scalar, z * scalar);
        }

        const Vec3 operator/(float scalar) const {
            return Vec3(x / scalar, y / scalar, z / scalar);
        }

        const Vec3 operator+(float scalar) const {
            return Vec3(x + scalar, y + scalar, z + scalar);
        }

        const float dot(const Vec3& other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        const Vec3 cross(const Vec3& other) const {
            return Vec3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }

        const Vec3 normalize() const {
            float length = std::sqrt(x * x + y * y + z * z);
            return Vec3(x / length, y / length, z / length);
        }
};