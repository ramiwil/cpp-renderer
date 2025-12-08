#pragma once

class vec3 {
    public:
        float x, y, z;
        vec3(float x, float y, float z) : x(x), y(y), z(z) {}
        vec3(float value) : x(value), y(value), z(value) {}

        const vec3 operator+(const vec3& other) const {
            return vec3(x + other.x, y + other.y, z + other.z);
        }

        const vec3 operator-(const vec3& other) const {
            return vec3(x - other.x, y - other.y, z - other.z);
        }

        const vec3 operator*(float scalar) const {
            return vec3(x * scalar, y * scalar, z * scalar);
        }

        const vec3 operator/(float scalar) const {
            return vec3(x / scalar, y / scalar, z / scalar);
        }

        const float dot(const vec3& other) const {
            return x * other.x + y * other.y + z * other.z;
        }
};