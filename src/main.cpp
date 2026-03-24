#include <omp.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "math/ray.h"
#include "math/utils.cpp"
#include "math/vec3.h"
#include "primitives/plane.h"
#include "primitives/sphere.h"
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

    // indirect sampling
    Material *mat = closest_obj->mat.get();
    Vec3 emitted = mat->emission * mat->emission_strength;

    constexpr float EPS = 1e-3;
    BxDF *bxdf = mat->get_bxdf();
    BxDFSample s = bxdf->sample(closest.normal, -ray.get_direction());
    Vec3 f = bxdf->evaluate(closest.normal, s.dir, -ray.get_direction());
    float cos_theta = closest.normal.dot(s.dir);

    Ray bounce_ray(closest.point + closest.normal * EPS, s.dir);
    Vec3 incoming = trace(bounce_ray, sc, depth - 1);

    // current points color
    return emitted + f * incoming * cos_theta / s.pdf;
}

Scene build_cornell_box() {
    Scene sc;

    // create sphere
    float radius = 18.0f;
    auto mat_sphere = std::make_shared<LambertianMaterial>(Vec3(1.0f));
    sc.add_object(std::make_unique<Sphere>(mat_sphere,
                                           Vec3(0.0f, -radius, 0.0f), radius));

    // create light
    auto mat_light = std::make_shared<LambertianMaterial>(Vec3(1.0f));
    mat_light->emission = Vec3(1.0f);
    mat_light->emission_strength = 20.0f;
    sc.add_object(std::make_unique<Plane>(mat_light, Vec3(0.0, 50.0f, 0.0),
                                          Vec3(0.0, 1.0, 0.0), 20.0f, 20.0f));

    // create box walls, floor, and ceiling
    float size = 100.0f;
    auto mat_white = std::make_shared<LambertianMaterial>(Vec3(1.0f));
    auto mat_red = std::make_shared<LambertianMaterial>(Vec3(1.0f, 0.0f, 0.0f));
    auto mat_green =
        std::make_shared<LambertianMaterial>(Vec3(0.0f, 1.0f, 0.0f));
    sc.add_object(std::make_unique<Plane>(mat_white, Vec3(0.0, size / 2.0, 0.0),
                                          Vec3(0.0, 1.0, 0.0), size,
                                          size));  // ceiling
    sc.add_object(
        std::make_unique<Plane>(mat_white, Vec3(0.0, -size / 2.0, 0.0),
                                Vec3(0.0, 1.0, 0.0), size, size));  // floor
    sc.add_object(std::make_unique<Plane>(mat_white, Vec3(0.0, 0.0, size / 2.0),
                                          Vec3(0.0, 0.0, 1.0), size,
                                          size));  // back wall
    sc.add_object(std::make_unique<Plane>(mat_red, Vec3(size / 2.0, 0.0f, 0.0f),
                                          Vec3(1.0, 0.0, 0.0), size,
                                          size));  // red wall
    sc.add_object(std::make_unique<Plane>(
        mat_green, Vec3(-size / 2.0, 0.0f, 0.0f), Vec3(1.0, 0.0, 0.0), size,
        size));  // green wall

    return sc;
}

int main() {
    Camera camera(Vec3(0.0f, 0.0f, -190.0),  // position
                  Vec3(0.0f, 0.0f, 0.0f),    // target
                  Vec3(0.0f, 1.0f, 0.0f),    // up
                  39.3f,                     // fov
                  WIDTH, HEIGHT);

    Scene scene = build_cornell_box();
    std::vector<Vec3> framebuffer(WIDTH * HEIGHT);

    // parameters
    int samples = 512;
    int depth = 4;

#pragma omp parallel for schedule(dynamic, 16)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Vec3 pixel_color(0.0f);
            for (int s = 0; s < samples; s++) {
                // small jitter to avoid antialiasing
                float u = (x + rand_float()) / float(WIDTH);
                float v = (y + rand_float()) / float(HEIGHT);
                Ray ray = camera.generate_ray(u, v);
                pixel_color += trace(ray, scene, depth);
            }
            pixel_color /=
                samples;  // average the final folor by the number of samples
            framebuffer[y * WIDTH + x] = pixel_color;
        }
    }

    // generate a png from the frame buffer
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