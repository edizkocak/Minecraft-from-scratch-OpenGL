#pragma once
#include "model.h"

class ScreenModelTest : public Model
{
public:
    using Base = Model;

    ScreenModelTest(globals::ModelType type,
                glm::vec3 pos = glm::vec3(0),
                glm::vec2 rot = glm::vec2(0),
                glm::vec3 scale = glm::vec3(1));
    void draw(Shader& shader);
    void update();

protected:
};