#pragma once

#include "model.h"

class MovableModel : public Model
{
public:
    using Base = Model;

    MovableModel(int id,
                 globals::ModelType type,
                 glm::vec3 pos = glm::vec3(0),
                 glm::vec2 rot = glm::vec2(0),
                 glm::vec3 scale = glm::vec3(1));

    // for active objects
    inline static int minId = -1, maxId = -1, nextId = -1;

    static std::map<int, std::shared_ptr<MovableModel>> movable_models;

    float hp = globals::hp;
    const int id;

protected:
    glm::vec3 velocity = glm::vec3(0);
};
