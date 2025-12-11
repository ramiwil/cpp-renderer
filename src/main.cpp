#include <iostream>
#include <fstream>
#include "vec3.h"
#include "sphere.h"
#include "utils.h"
#include "camera.h"
#include "ray.h"
#include "scene.h"

constexpr int WIDTH = 512;
constexpr int HEIGHT = 512;


int main() {
    std::ofstream render("output.ppm", std::ios::binary);
    if (!render) {
        std::cerr << "Error creating output file!" << std::endl;
        return -1;
    }

    render << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";

    Camera camera(
        Vec3(0.0f, 0.0f, -10.0),    // position
        Vec3(0.0f, 0.0f, 5.0f),     // target
        Vec3(0.0f, 1.0f, 0.0f),     // up
        90.0f,                      // fov
        WIDTH,
        HEIGHT
    );

    Material mat_1{Vec3(255.0f, 255.0f, 0.0f)};
    Material mat_2{Vec3(0.0f, 255.0f, 255.0f)};

    Scene scene;
    scene.add_object(std::make_unique<Sphere>(mat_1, Vec3(0.0f, 0.0f, 0.0f), 1.0f));
    scene.add_object(std::make_unique<Sphere>(mat_2, Vec3(0.0f, 1.0f, -2.0f), 1.0f));

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float u = float(x) / float(WIDTH - 1); // normalized coordinates
            float v = float(y) / float(HEIGHT - 1); // normalized coordinates

            Ray ray = camera.generate_ray(u, v);
            Vec3 pixel_color(0.0f, 0.0f, 0.0f);
            float closest_hit = std::numeric_limits<float>::max();

            for (auto &obj : scene.objects) {
                hit_result result = obj->hit(ray); 
                if (result.hit && (result.t < closest_hit)) {
                    closest_hit = result.t;
                    pixel_color = obj->get_material().color;
                }
            }

            write_color(render, pixel_color);
        }
    }

    return 0;
}