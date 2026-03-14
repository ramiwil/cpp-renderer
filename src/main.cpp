#include <omp.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "math/ray.h"
#include "math/utils.h"
#include "math/vec3.h"
#include "primitives/back_wall.h"
#include "primitives/planes.h"
#include "primitives/sphere.h"
#include "primitives/wall.h"
#include "scene/camera.h"
#include "scene/scene.h"
#include "shading/material.h"
#include "stb_image_write.h"

constexpr int WIDTH = 128;
constexpr int HEIGHT = 128;

Vec3 compute_color(Vec3 hit_point, Vec3 hit_normal, Vec3 view_dir, Material mat,
                   const Scene &sc) {
    Vec3 shade(0.0f);
    Vec3 ambient = mat.diffuse * 0.1f;
    shade += ambient;

    constexpr float EPS = 1e-4;
    Vec3 shadow_origin = hit_point + hit_normal * EPS;

    for (auto &light : sc.lights) {
        Vec3 to_light = light->get_position() - hit_point;
        float light_distance = to_light.length();
        Vec3 light_dir = to_light / light_distance;
        float attenuation = 1.0f / (light_distance * light_distance);

        Ray shadow_ray(shadow_origin, light_dir);

        bool in_shadow = false;

        for (auto &obj : sc.objects) {
            hit_result shadow_hit = obj->hit(shadow_ray);
            if (shadow_hit.hit && shadow_hit.t < light_distance) {
                in_shadow = true;
                break;
            }
        }

        if (in_shadow) {
            continue;
        }

        // diffuse component
        float ndotl = std::max(0.0f, hit_normal.dot(light_dir));
        Vec3 diffuse =
            mat.diffuse * light->get_intensity() * attenuation * ndotl;
        shade += diffuse;

        // specular component
        Vec3 to_camera = -view_dir.normalize();
        Vec3 half_vec = (light_dir + to_camera).normalize();
        float spec_angle = std::max(0.0f, hit_normal.dot(half_vec));
        float spec_intensity = std::pow(spec_angle, mat.shininess);
        Vec3 specular = mat.specular * light->get_intensity() * attenuation *
                        spec_intensity;
        shade += specular;
    }

    return shade * mat.color;
}

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
    Vec3 bounce_dir = random_dir(closest.normal);
    Ray bounce_ray(closest.point + closest.normal * EPS, bounce_dir);

    return emitted + mat.color * trace(bounce_ray, sc, depth - 1);
}

Scene build_scene() {
    Scene sc;

    Material mat_1{Vec3(1.0f, 0.0f, 0.0f)};
    Material mat_2{Vec3(0.0f, 1.0f, 1.0f)};
    Material mat_3{Vec3(1.0f, 1.0f, 1.0f)};
    mat_3.emission = Vec3(1.0f, 1.0f, 1.0f);
    mat_3.emission_strength = 20.0f;

    sc.add_object(
        std::make_unique<Sphere>(mat_1, Vec3(0.0f, -40.0f, 0.0f), 10.0f));
    sc.add_object(
        std::make_unique<Sphere>(mat_2, Vec3(-10.0f, -40.0f, 0.0f), 3.0f));

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
    Camera camera(Vec3(0.0f, 0.0f, -200.0),  // position
                  Vec3(0.0f, 0.0f, 5.0f),    // target
                  Vec3(0.0f, 1.0f, 0.0f),    // up
                  39.3f,                     // fov
                  WIDTH, HEIGHT);

    Scene scene = build_scene();

    std::vector<Vec3> framebuffer(WIDTH * HEIGHT);

#pragma omp parallel for num_threads(16)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Vec3 pixel_color(0.0f);
            int samples = 512;
            int depth = 8;
            for (int s = 0; s < samples; s++) {
                float u = (x + rand_float()) / float(WIDTH);
                float v = (y + rand_float()) / float(HEIGHT);
                Ray ray = camera.generate_ray(u, v);
                pixel_color += trace(ray, scene, depth);
            }
            pixel_color /= samples;  // average number of samples
            framebuffer[y * WIDTH + x] = pixel_color;

            // float closest_hit = std::numeric_limits<float>::max();
            // Vec3 shade(0.0f);
            // for (auto &obj : scene.objects) {
            //     hit_result result = obj->hit(ray);
            //     if (result.hit && (result.t < closest_hit)) {
            //         closest_hit = result.t;
            //         Material mat = obj->get_material();

            //         pixel_color = compute_color(
            //             result.point,
            //             result.normal,
            //             ray.get_direction(),
            //             mat,
            //             scene
            //         );
            //     }
            // }
            // write_color(render, pixel_color);
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