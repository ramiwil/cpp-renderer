#include <iostream>
#include <fstream>
#include "vec3.h"
#include "sphere.h"
#include "utils.h"

constexpr int WIDTH = 500;
constexpr int HEIGHT = 500;


int main() {
    std::ofstream render("output.ppm", std::ios::binary);
    if (!render) {
        std::cerr << "Error creating output file!" << std::endl;
        return -1;
    }

    render << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float u = float(x) / float(WIDTH - 1); // normalized coordinates
            float v = float(y) / float(HEIGHT - 1); // normalized coordinates

            // Replace this with your raytracer's color for pixel (u,v)
            vec3 color = vec3(u, v, 155.2f);

            write_color(render, color);
        }
    }

    return 0;
}