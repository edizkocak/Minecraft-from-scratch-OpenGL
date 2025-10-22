#ifndef GRAPA_ANIMATED_MODEL_H
#define GRAPA_ANIMATED_MODEL_H

#include "movable_model.h"
#include "../animation/animator.h"

class AnimatedModel : public MovableModel{
    using Base = MovableModel;

public:
    AnimatedModel(int id,
                 globals::ModelType type,
                 glm::vec3 pos = glm::vec3(0),
                 glm::vec2 rot = glm::vec2(0),
                 glm::vec3 scale = glm::vec3(1));

    static std::shared_ptr<AnimatedModel> newAnimatedModel(int id, globals::ModelType type, glm::vec3 pos);

    void loadAnimated();

    bool dead = false;  // TODO animals "die" in the night ??

    static void add_animated_model(glm::ivec3 pos, globals::ModelType type);

    void update() override;

    void draw(Shader &shader) override;

    void changeAnimation(globals::AnimationType aType, int offset);

    void set_speed_factor(float speed_fac);

    globals::AnimationType aType;

protected:
    std::map<globals::AnimationType, Animation1> as; // TODO static ?

    Timer aTimer;

    SkelAnimation::Model skelModel;
    Animator animator;
};

#endif //GRAPA_ANIMATED_MODEL_H
