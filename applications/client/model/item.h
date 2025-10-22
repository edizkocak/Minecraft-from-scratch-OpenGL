#pragma once
#include "animated_model.h"

class Item : public AnimatedModel
{
public:
    using Base = AnimatedModel;

    explicit Item(int id, glm::ivec3 pos, globals::ModelType type);
    void update() override;
   // void draw(Shader &shader) override;

protected:

};



