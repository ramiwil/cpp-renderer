#include <iostream>
#include <fstream>
#include "vec3.h"
#include "sphere.h"
#include "planes.h"
#include "utils.h"
#include "camera.h"
#include "ray.h"
#include "scene.h"

constexpr int WIDTH = 1024;
constexpr int HEIGHT = 1024;

void debug(const Vec3& vec) {
    std::cout << vec.x << ' ' << vec.y << ' ' << vec.z << std::endl;
}

void debug(const float& val) {
    std::cout << val << std::endl;
}

Vec3 compute_color(Vec3 hit_point, Vec3 hit_normal, Vec3 view_dir, Material mat, const Scene& sc) {
    Vec3 shade(0.0f);
    Vec3 ambient = mat.diffuse * 0.1f;
    shade += ambient;

    constexpr float EPS = 1e-4;
    Vec3 shadow_origin = hit_point + hit_normal * EPS;

    for (auto &light: sc.lights) {
        Vec3 to_light = light->get_position() - hit_point;
        float light_distance = to_light.length();
        Vec3 light_dir = to_light / light_distance;
        float attenuation = 1.0f / (light_distance * light_distance);

        Ray shadow_ray(shadow_origin, light_dir);

        bool in_shadow = false;

        for (auto &obj: sc.objects) {
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
        Vec3 diffuse = mat.diffuse * light->get_intensity() * attenuation * ndotl;
        shade += diffuse;

        // specular component
        Vec3 to_camera = -view_dir.normalize();
        Vec3 half_vec = (light_dir + to_camera).normalize();
        float spec_angle = std::max(0.0f, hit_normal.dot(half_vec));
        float spec_intensity = std::pow(spec_angle, mat.shininess);
        Vec3 specular = mat.specular * light->get_intensity() * attenuation * spec_intensity;
        shade += specular;
    }

    return shade * mat.color;
}

int main() {
    std::ofstream render("../renders/output.ppm", std::ios::binary);
    if (!render) {
        std::cerr << "Error creating output file!" << std::endl;
        return -1;
    }

    render << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";

    Camera camera(
        Vec3(0.0f, 0.0f, -200.0),    // position
        Vec3(0.0f, 0.0f, 5.0f),     // target
        Vec3(0.0f, 1.0f, 0.0f),     // up
        39.3f,                      // fov
        WIDTH,
        HEIGHT
    );

    Material mat_1{Vec3(1.0f, 0.0f, 0.0f)};
    Material mat_2{Vec3(0.0f, 1.0f, 1.0f)};
    Material mat_3{Vec3(1.0f, 1.0f, 1.0f)};

    Scene scene;
    scene.add_object(std::make_unique<Sphere>(mat_1, Vec3(0.0f, 0.0f, 0.0f), 10.0f));
    scene.add_object(std::make_unique<Sphere>(mat_2, Vec3(-10.0f, 10.0f, 0.0f), 3.0f));


    // box
    float size = 100.0f;
    scene.add_object(std::make_unique<XZRect>(mat_3, Vec3(0.0, -(size / 2.0), 0.0), size, size)); // bottom
    scene.add_object(std::make_unique<XZRect>(mat_3, Vec3(0.0, (size / 2.0), 0.0), size, size)); // top

    scene.add_light(std::make_unique<Light>(Vec3(0.0f, (size / 2.0)-1.0, 0.0f), 1000.0f));

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float u = float(x) / float(WIDTH - 1); // normalized coordinates
            float v = float(y) / float(HEIGHT - 1); // normalized coordinates

            Ray ray = camera.generate_ray(u, v);
            Vec3 shade(0.0f);
            Vec3 pixel_color(0.0f);
            float closest_hit = std::numeric_limits<float>::max();

            for (auto &obj : scene.objects) {
                hit_result result = obj->hit(ray); 
                if (result.hit && (result.t < closest_hit)) {
                    closest_hit = result.t;
                    Material mat = obj->get_material();
                    
                    pixel_color = compute_color(
                        result.point, 
                        result.normal, 
                        ray.get_direction(), 
                        mat, 
                        scene
                    );
                }
            }
            write_color(render, pixel_color);
        }
    }

    return 0;
}