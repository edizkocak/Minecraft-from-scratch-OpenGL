#pragma once
#include "animated_model.h"

class ScreenModel : public AnimatedModel
{
public:
    using Base = AnimatedModel;

    ScreenModel(int id, globals::ModelType type,
                 glm::vec3 pos);
    void draw(Shader& shader);
    virtual void update();

    float anim_speed_fac;
    bool animation_active = false;

    bool cooldown_active = false;
    Timer cooldown_timer;
    Timer hit_timer;
    float hit_time = 50.f;
    float animation_cooldown = 0.f;

    float damage = 0.f;
    float radius = 0.f;

protected:
};


class Arm : public ScreenModel {
public:
    using Base = ScreenModel;

    explicit Arm(int id);

    void update();
};


class Sword : public ScreenModel {
public:
    using Base = ScreenModel;

    explicit Sword(int id);
};


class Axe : public ScreenModel {
public:
    using Base = ScreenModel;

    explicit Axe(int id);
};