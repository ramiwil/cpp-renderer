#include <cmath>
#include "vec3.h"
#include "ray.h"

class Camera {
    public:
        Camera(const Vec3& position, const Vec3& target, const Vec3& world_up, float fov, int width, int height):
            position(position), 
            target(target), 
            world_up(world_up), 
            fov(fov), 
            width(width), 
            height(height), 
            aspect_ratio(float(width) / float(height)),
            camera_up(0.0f),
            forward(0.0f),
            right(0.0f)
        {
            updateBases();
        };

        Ray generate_ray(float x, float y) const {
            float theta = fov * M_PI / 100.0f;
            float half_height = std::tan(theta / 2.0f);
            float half_width = aspect_ratio * half_height;

            float u = (2 * x - 1) * half_width;
            float v = (1 - 2 * y) * half_height;

            Vec3 ray_direction = (forward + (right*u) + (camera_up*v)).normalize();
            return Ray(position, ray_direction);
        };

    private:
        void updateBases() {
            forward = (target - position).normalize();
            right = forward.cross(world_up).normalize();
            camera_up = right.cross(forward).normalize();
        }

        Vec3 position;
        Vec3 target;
        Vec3 world_up;

        Vec3 camera_up;
        Vec3 forward;
        Vec3 right;

        float fov;
        int width;
        int height;
        float aspect_ratio;
};