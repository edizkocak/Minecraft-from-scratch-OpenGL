#pragma once
#include "animated_model.h"

class Animal : public AnimatedModel
{
public:
    using Base = AnimatedModel;

    explicit Animal(int id, glm::ivec3 pos, globals::ModelType type);
    void update() override;

protected:

};