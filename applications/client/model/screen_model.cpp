#include "screen_model.h"
#include "model.h"
#include "player.h"

extern float rx;
extern float ry;
extern float rz;

extern float xa;
extern float ya;
extern float za;

extern float s;

extern std::shared_ptr<Player> the_player;



ScreenModel::ScreenModel(int id, globals::ModelType type,
                           glm::vec3 pos = glm::vec3(0)
                           )
        : Base(id, type, pos, glm::vec2(0), glm::vec3(1.f))
{
    set_speed_factor(0.f);
}


void ScreenModel::draw(Shader& shader){
    Base::draw(shader);
}

void ScreenModel::update(){
    //animator.speedfactor = s;
    if(animation_active){
        set_speed_factor(anim_speed_fac);
    }

    animator.UpdateAnimation(float(aTimer.look()));

    if(animator.animation_finished){
        animator.animation_finished = false;
        changeAnimation(globals::AnimationType::WALKING, 0);
        set_speed_factor(0.f);

        cooldown_timer.begin();
        cooldown_active = true;
    }

    animation_active = false;

//    model = glm::mat4(1);
//
//    //yaw
//    model = glm::rotate(model, rx, VEC_UP);
//
//    //pitch
//    //model = glm::rotate(model, 2.519f, VEC_RIGHT);
//    model = glm::rotate(model, ry, VEC_RIGHT);
//
//    //roll
//    model = glm::rotate(model, rz, VEC_FORWARD);
//
//    // set pos
//    model[3][0] = xa;
//    model[3][1] = ya;
//    model[3][2] = za;
}


Arm::Arm(int id)
        : Base(id, globals::ModelType::arm) {
    model = glm::mat4(1);

    //yaw
    model = glm::rotate(model, 2.108f, VEC_UP);

    //pitch
    //model = glm::rotate(model, 2.519f, VEC_RIGHT);
    model = glm::rotate(model, 2.519f, VEC_RIGHT);

    //roll
    model = glm::rotate(model, -0.566f, VEC_FORWARD);

    // set pos
    model[3][0] = 0.874f;
    model[3][1] = 0.f;
    model[3][2] = -1.78f;

    anim_speed_fac = 0.03f;
    damage = 10.f;
    radius = 3.f;
}

void Arm::update(){
    if(animation_active){
        set_speed_factor(anim_speed_fac);
    }

    animator.UpdateAnimation(float(aTimer.look()));

    if(animator.animation_finished){
        animator.animation_finished = false;
        if(!animation_active) {
            changeAnimation(globals::AnimationType::WALKING, 0);
            set_speed_factor(0.f);
        }
    }

    animation_active = false;
}


Sword::Sword(int id)
        : Base(id, globals::ModelType::sword_screen) {
    model = glm::mat4(1);

    //yaw
    model = glm::rotate(model, 3.006f, VEC_UP);

    //pitch
    //model = glm::rotate(model, 2.519f, VEC_RIGHT);
    model = glm::rotate(model, 3.225f, VEC_RIGHT);

    //roll
    model = glm::rotate(model, 4.295f, VEC_FORWARD);

    // set pos
    model[3][0] = 0.416f;
    model[3][1] = -0.932f;
    model[3][2] = -1.276f;

    anim_speed_fac = 0.015f;
    animation_cooldown = 0.5f;
    hit_time = 170.f;

    damage = 25.f;
    radius = 5.f;
}


Axe::Axe(int id)
        : Base(id, globals::ModelType::axe_screen) {
    model = glm::mat4(1);

    //yaw
    model = glm::rotate(model, 3.141f, VEC_UP);

    //pitch
    //model = glm::rotate(model, 2.519f, VEC_RIGHT);
    model = glm::rotate(model, 3.427f, VEC_RIGHT);

    //roll
    model = glm::rotate(model, 4.415f, VEC_FORWARD);

    // set pos
    model[3][0] = 0.158f;
    model[3][1] = -0.588f;
    model[3][2] = -1.161f;

    anim_speed_fac = 0.0045f;
    animation_cooldown = 0.5f;
    hit_time = 500.f;

    damage = 40.f;
    radius = 4.5f;
}



