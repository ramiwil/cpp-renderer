#include <cmath>
#include <cstdlib>
#include <omp.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "math/ray.h"
#include "math/utils.cpp"
#include "math/vec3.h"
#include "primitives/back_wall.h"
#include "primitives/planes.h"
#include "primitives/sphere.h"
#include "primitives/wall.h"
#include "scene/camera.h"
#include "scene/scene.h"
#include "shading/material.h"
#include "stb_image_write.h"

constexpr int WIDTH = 512;
constexpr int HEIGHT = 512;

Vec3 trace(Ray ray, const Scene &sc, int depth) {
    if (depth == 0) return Vec3{0.0f};

    hit_result closest;
    Object *closest_obj = nullptr;
    float min_t = std::numeric_limits<float>::max();
    for (auto &obj : sc.objects) {
        auto res = obj->hit(ray);
        if (res.hit && res.t < min_t) {
            min_t = res.t;
            closest = res;
            closest_obj = obj.get();
        }
    }

    if (!closest_obj) return Vec3{0.0f};

    Material mat = closest_obj->get_material();
    Vec3 emitted = mat.emission * mat.emission_strength;

    // randome diffuse bounce
    constexpr float EPS = 1e-4;
    Vec3 bounce_dir;

    if ((rand() / float(RAND_MAX)) < mat.reflectivity) {
        float dot_product = ray.get_direction().dot(closest.normal);
        bounce_dir = (ray.get_direction() - (2 * dot_product * closest.normal)).normalize();
    } else {
        bounce_dir = random_dir(closest.normal);
    }
    Ray bounce_ray(closest.point + closest.normal * EPS, bounce_dir);

    return emitted + mat.color * trace(bounce_ray, sc, depth - 1);
}

Scene build_scene() {
    Scene sc;

    Material mat_1{Vec3(1.0f, 1.0f, 1.0f)};
    mat_1.reflectivity = 0.1f;
    // Material mat_2{Vec3(0.0f, 1.0f, 1.0f)};
    Material mat_3{Vec3(1.0f, 1.0f, 1.0f)};
    mat_3.emission = Vec3(1.0f, 1.0f, 1.0f);
    mat_3.emission_strength = 20.0f;

    float radius = 18.0f;
    sc.add_object(
        std::make_unique<Sphere>(mat_1, Vec3(0.0f, -radius, 0.0f), radius));
    // sc.add_object(
    //     std::make_unique<Sphere>(mat_2, Vec3(-10.0f, -40.0f, 0.0f), 3.0f));

    // box
    float size = 100.0f;
    sc.add_object(std::make_unique<XZRect>(mat_3, Vec3(0.0, (size / 2.0), 0.0),
                                           20.0f, 20.0f));

    sc.add_object(std::make_unique<XZRect>(
        Material{}, Vec3(0.0, (size / 2.0), 0.0), size, size));  // top
    sc.add_object(std::make_unique<XZRect>(
        Material{}, Vec3(0.0, -(size / 2.0), 0.0), size, size));  // bottom

    sc.add_object(std::make_unique<YXRect>(
        Material{}, Vec3(0.0, 0.0, (size / 2.0)), size, size));  // back wall

    Material wall_1{};
    wall_1.color = Vec3(1.0f, 0.0f, 0.0f);
    sc.add_object(std::make_unique<YZRect>(
        wall_1, Vec3((size / 2.0), 0.0f, 0.0f), size, size));

    Material wall_2{};
    wall_2.color = Vec3(0.0f, 1.0f, 0.0f);
    sc.add_object(std::make_unique<YZRect>(
        wall_2, Vec3(-(size / 2.0), 0.0f, 0.0f), size, size));

    return sc;
}

int main() {
    Camera camera(Vec3(0.0f, 0.0f, -190.0),  // position
                  Vec3(0.0f, 0.0f, 5.0f),    // target
                  Vec3(0.0f, 1.0f, 0.0f),    // up
                  39.3f,                     // fov
                  WIDTH, HEIGHT);

    Scene scene = build_scene();

    std::vector<Vec3> framebuffer(WIDTH * HEIGHT);

    #pragma omp parallel for schedule(dynamic, 5)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Vec3 pixel_color(0.0f);
            int samples = 256;
            int depth = 4;
            for (int s = 0; s < samples; s++) {
                float u = (x + rand_float()) / float(WIDTH);
                float v = (y + rand_float()) / float(HEIGHT);
                Ray ray = camera.generate_ray(u, v);
                pixel_color += trace(ray, scene, depth);
            }
            pixel_color /= samples;  // average number of samples
            framebuffer[y * WIDTH + x] = pixel_color;
        }
    }

    std::vector<uint8_t> pixels(WIDTH * HEIGHT * 3);
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        pixels[i * 3 + 0] = color_to_byte(framebuffer[i].x);
        pixels[i * 3 + 1] = color_to_byte(framebuffer[i].y);
        pixels[i * 3 + 2] = color_to_byte(framebuffer[i].z);
    }

    stbi_write_png("renders/output.png", WIDTH, HEIGHT, 3, pixels.data(),
                   WIDTH * 3);

    return 0;
}