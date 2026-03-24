#pragma once
#include <memory>
#include <vector>

#include "primitives/object.h"
#include "scene/light.h"

class Scene {
  public:
    std::vector<std::unique_ptr<Object>> objects;
    std::vector<Light *> lights;

    void add_object(std::unique_ptr<Object> obj) {
        objects.push_back(std::move(obj));
    }

    void add_light(std::unique_ptr<Light> light) {
        Light *ptr = light.get();
        objects.push_back(std::move(light));
        lights.push_back(ptr);
    }
};
