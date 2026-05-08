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
#include "gpu_render.h"


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

    Material *mat = closest_obj->mat.get();
    Vec3 emitted = mat->emission * mat->emission_strength;

    constexpr float EPS = 1e-3;
    BxDF *bxdf = mat->get_bxdf();

    // direct sampling (NEE)
    Vec3 f_direct(0.0f);
    for (auto &light : sc.lights) {
        Vec3 light_point = light->sample();
        Vec3 to_light = light_point - closest.point;
        float dist_sq = to_light.dot(to_light);
        float dist = std::sqrt(dist_sq);
        Vec3 dir_to_light = to_light / dist;

        Ray shadow_ray(closest.point + closest.normal * EPS, dir_to_light);
        bool occluded = false;
        for (auto &obj : sc.objects) {
            auto res = obj->hit(shadow_ray);
            if (res.hit && res.t < dist - 2 * EPS) {
                occluded = true;
                break;
            }
        }

        if (!occluded) {
            float cos_surface = closest.normal.dot(dir_to_light);
            float cos_light = light->normal.dot(dir_to_light);
            if (cos_surface > 0 && cos_light > 0) {
                Material *light_mat = light->mat.get();
                Vec3 Le = light_mat->emission * light_mat->emission_strength;
                Vec3 f = bxdf->evaluate(closest.normal, dir_to_light,
                                        -ray.get_direction());
                f_direct += f * Le * cos_surface * cos_light /
                            (dist_sq * light->pdf_area());
            }
        }
    }

    // indirect sampling
    BxDFSample s = bxdf->sample(closest.normal, -ray.get_direction());
    Vec3 f_indirect =
        bxdf->evaluate(closest.normal, s.dir, -ray.get_direction());
    float cos_theta = closest.normal.dot(s.dir);

    Ray bounce_ray(closest.point + closest.normal * EPS, s.dir);
    Vec3 incoming = trace(bounce_ray, sc, depth - 1);

    return emitted + f_direct + f_indirect * incoming * cos_theta / s.pdf;
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
    mat_light->emission_strength = 10.0f;
    sc.add_light(std::make_unique<Light>(mat_light, Vec3(0.0, 49.9, 0.0),
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
    scene_params sp;
    Camera camera(Vec3(0.0f, 0.0f, -190.0),  // position
                Vec3(0.0f, 0.0f, 0.0f),    // target
                Vec3(0.0f, 1.0f, 0.0f),    // up
                39.3f,                     // fov
                sp.width, sp.height);
    Scene scene = build_cornell_box();

    float* frame_buffer = render_gpu(
        camera, 
        scene,
        sp
    );

    // write to png
    std::vector<uint8_t> out(sp.width * sp.height * 3);
    for (int i = 0; i < sp.width * sp.height * 3; i++) {
        out[i] = float_to_color(frame_buffer[i]);
    }

    stbi_write_png("renders/output_gpu.png", sp.width, sp.height, 3, out.data(), sp.width * 3);

    return 0;
}