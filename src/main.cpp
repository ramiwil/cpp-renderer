#include <iostream>
#include <fstream>
#include "vec3.h"
#include "sphere.h"
#include "utils.h"
#include "camera.h"
#include "ray.h"

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

    Sphere sphere(Vec3(0.0f, 0.0f, 0.0f), 1.0f);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float u = float(x) / float(WIDTH - 1); // normalized coordinates
            float v = float(y) / float(HEIGHT - 1); // normalized coordinates

            Ray ray = camera.generate_ray(u, v);

            hit_result hit = sphere.hit(ray);

            if (hit.hit == true) {
                Vec3 color = Vec3(255.0f, 0.0f, 0.0f);
                write_color(render, color);
            } else {
                Vec3 color = Vec3(0.0f, 0.0f, 0.0f);
                write_color(render, color);
            }
        }
    }

    return 0;
}