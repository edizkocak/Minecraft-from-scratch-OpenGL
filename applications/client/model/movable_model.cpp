#include "movable_model.h"
#include "model.h"

std::map<int, std::shared_ptr<MovableModel>>
MovableModel::movable_models;

MovableModel::MovableModel(int id,
                           globals::ModelType type,
                           glm::vec3 pos,
                           glm::vec2 rot,
                           glm::vec3 scale)
        : Base(type, pos, rot, scale), id(id)
{
}

