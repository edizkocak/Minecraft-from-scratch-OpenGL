#pragma once
#include "animated_model.h"

class Enemy : public AnimatedModel
{
public:
    using Base = AnimatedModel;

    explicit Enemy(int id, glm::ivec3 pos, globals::ModelType type);
    void update() override;

    bool enemy_is_hit = false;

    float redness_factor = 0.f;

    Timer hit_redness_timer;

protected:

};
