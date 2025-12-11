#pragma once
#include <vector>
#include <memory>
#include "object.h"

class Scene {
public:
    std::vector<std::unique_ptr<Object>> objects;

    void add_object(std::unique_ptr<Object> obj) {
        objects.push_back(std::move(obj));
    }
};
