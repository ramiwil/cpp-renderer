#include "vec3.h"

class Light {
    public:
        Light(Vec3 pos, float intensity) : position(pos), intensity(intensity) {}
        Vec3 get_position() {
            return position;
        }
        float get_intensity() {
            return intensity;
        }
    
    private:
        Vec3 position;
        float intensity;
};