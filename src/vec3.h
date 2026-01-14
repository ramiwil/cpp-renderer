#pragma once
#include <cmath>

class Vec3 {
    public:
        float x, y, z;
        Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
        Vec3(float value) : x(value), y(value), z(value) {}

        Vec3 operator+(const Vec3& other) const {
            return Vec3(x + other.x, y + other.y, z + other.z);
        }

        void operator+=(const Vec3& other) {
            x = x + other.x;
            y = y + other.y;
            z = z + other.z;
        }

        Vec3 operator-(const Vec3& other) const {
            return Vec3(x - other.x, y - other.y, z - other.z);
        }

        Vec3 operator*(const Vec3& other) const {
            return Vec3(x*other.x, y*other.y, z*other.z);
        }

        Vec3 operator*(float scalar) const {
            return Vec3(x * scalar, y * scalar, z * scalar);
        }

        Vec3 operator/(float scalar) const {
            return Vec3(x / scalar, y / scalar, z / scalar);
        }

        Vec3 operator+(float scalar) const {
            return Vec3(x + scalar, y + scalar, z + scalar);
        }

        Vec3 operator-() const {
            return Vec3(-x, -y, -z);  // returns opposite of direction
        }

        float dot(const Vec3& other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        Vec3 cross(const Vec3& other) const {
            return Vec3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }

        Vec3 normalize() const {
            float length = std::sqrt(x * x + y * y + z * z);
            return Vec3(x / length, y / length, z / length);
        }

        float length() const {
            return std::sqrt(x * x + y * y + z * z);
        }
};