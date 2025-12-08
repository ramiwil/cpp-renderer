#include <algorithm>
#include <cmath>
#include "vec3.h"

unsigned char color_to_byte(float c) {
    float gamma_corrected = std::sqrt(std::clamp(c, 0.0f, 1.0f));
    return static_cast<unsigned char>(gamma_corrected * 255.0f);
}


void write_color(std::ostream& out, vec3& color) {
    out.put(color_to_byte(color.x));
    out.put(color_to_byte(color.y));
    out.put(color_to_byte(color.z));
}