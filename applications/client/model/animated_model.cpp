#include "animated_model.h"
#include "enemy.h"
#include "player.h"
#include "animal.h"

AnimatedModel::AnimatedModel(int id,
                             globals::ModelType type,
                             glm::vec3 pos,
                             glm::vec2 rot,
                             glm::vec3 scale)
        : Base(id, type, pos, rot, scale) // TODO don't load model twice
{
    aType = globals::AnimationType::WALKING;
    aTimer.begin();

    // std::cout << "anim_constructor" << std::endl;

    loadAnimated();
}

std::shared_ptr<AnimatedModel> AnimatedModel::newAnimatedModel(int id, globals::ModelType type, glm::vec3 pos) {
    pos += glm::vec3{0.5, 0, 0.5};

    switch(type){
        case globals::ModelType::player:
            return make_shared<Player>(id, pos, 0);
        case globals::ModelType::dog:
        case globals::ModelType::horse:
            return make_shared<Animal>(id, pos, type);
        case globals::ModelType::zombie:
        case globals::ModelType::skeleton:
            return make_shared<Enemy>(id, pos, type);

        case globals::ModelType::air:
        case globals::ModelType::water:
        case globals::ModelType::wood:
        case globals::ModelType::dirt:
        case globals::ModelType::grass:
        case globals::ModelType::leaf:
        case globals::ModelType::stone:
        case globals::ModelType::torch:
        case globals::ModelType::cloud0:
        case globals::ModelType::cloud1:
        case globals::ModelType::cloud2:
        case globals::ModelType::cloud3:
        case globals::ModelType::sword_screen:
        case globals::ModelType::axe_screen:
        case globals::ModelType::sword:
        case globals::ModelType::axe:
        case globals::ModelType::arm:
            break;
    }
    throw std::runtime_error("not an animated model");
}

void AnimatedModel::loadAnimated() {
    fs::path file;

    switch(type){
        case globals::ModelType::air:
        case globals::ModelType::water:
        case globals::ModelType::wood:
        case globals::ModelType::dirt:
        case globals::ModelType::grass:
        case globals::ModelType::leaf:
        case globals::ModelType::stone:
        case globals::ModelType::torch:
        case globals::ModelType::cloud0:
        case globals::ModelType::cloud1:
        case globals::ModelType::cloud2:
        case globals::ModelType::cloud3:
            throw std::runtime_error("not an animated model");

        case globals::ModelType::player:
            file = globals::renderData() / "animations" / "C_walking.dae";
            skelModel = SkelAnimation::Model(file, false);
            as[globals::AnimationType::STANDING] = Animation1(globals::renderData() / "animations" / "C_standing1.dae", skelModel);
            as[globals::AnimationType::DIGGING] = Animation1(globals::renderData() / "animations" / "C_digging_with_hand.dae", skelModel); // TODO shared ptr or &
            as[globals::AnimationType::HITTING] = Animation1(globals::renderData() / "animations" / "C_hitting.dae", skelModel); 
            break;
        case globals::ModelType::dog:
            file = globals::renderData() / "animations" / "dog.dae";
            skelModel = SkelAnimation::Model(file, false);
            break;
        case globals::ModelType::horse:
            file = globals::renderData() / "animations" / "horse.dae";
            skelModel = SkelAnimation::Model(file, false);
            break;
        case globals::ModelType::zombie:
            file = globals::renderData() / "animations" / "zombie0.dae";
            skelModel = SkelAnimation::Model(file, false);
            break;
        case globals::ModelType::skeleton:
            file = globals::renderData() / "animations" / "skeleton.dae";
            skelModel = SkelAnimation::Model(file, false);
            break;
        case globals::ModelType::arm:
            file = globals::renderData() / "animations" / "hand_digging_on_screen.dae";
            skelModel = SkelAnimation::Model(file, false);
            break;
        case globals::ModelType::sword_screen:
            file = globals::renderData() / "animations" / "sword_hitting_on_screen0.dae";
            skelModel = SkelAnimation::Model(file, false);
            break;
        case globals::ModelType::axe_screen:
            file = globals::renderData() / "animations" / "axe_hitting_on_screen.dae";
            skelModel = SkelAnimation::Model(file, false);
            break;
        case globals::ModelType::sword:
            file = globals::renderData() / "animations" / "sword_walking.dae";
            skelModel = SkelAnimation::Model(file, false);
            as[globals::AnimationType::STANDING] = Animation1(globals::renderData() / "animations" / "sword_standing.dae", skelModel);
            as[globals::AnimationType::HITTING] = Animation1(globals::renderData() / "animations" / "sword_hitting.dae", skelModel);
            break;
        case globals::ModelType::axe:
            file = globals::renderData() / "animations" / "axt_walking.dae";
            skelModel = SkelAnimation::Model(file, false);
            as[globals::AnimationType::STANDING] = Animation1(globals::renderData() / "animations" / "axt_standing.dae", skelModel);
            as[globals::AnimationType::HITTING] = Animation1(globals::renderData() / "animations" / "axe_hitting.dae", skelModel);
            break;
    }


    as[globals::AnimationType::WALKING] = Animation1(file, skelModel); // TODO shared ptr or &
    animator = Animator(&as[globals::AnimationType::WALKING], 0.01);
    aType = globals::AnimationType::WALKING;
}


void AnimatedModel::add_animated_model(glm::ivec3 pos, globals::ModelType type) {
//    Message msg(Message::Type::CL_ADD_MOVABLE);
//    msg.add("movableObjId", nextId);
//    msg.add("MovableObjType", type);
//    Server::add_msge(msg);

    if( globals::ModelType::zombie == type ||  globals::ModelType::skeleton == type){
        MovableModel::movable_models[nextId] = make_shared<Enemy>(nextId, pos, type);
    }
    else if(type == globals::ModelType::player) {
        MovableModel::movable_models[nextId] = make_shared<Player>(nextId, pos, 0);
    }
    else if(type == globals::ModelType::dog || type == globals::ModelType::horse){
        MovableModel::movable_models[nextId] = make_shared<Animal>(nextId, pos, type);
    }

    nextId++;
}

void AnimatedModel::update() {
    Base::update();
    animator.UpdateAnimation(float(aTimer.look()));
}

void AnimatedModel::draw(Shader &shader) {
    if(!visible) return;
    if(type == globals::ModelType::axe || type == globals::ModelType::sword){
        return;
    }

    bool hit_uniform = false;
    float redness_f = 0.0;
    if(dynamic_cast<Enemy *>(this)){
        hit_uniform = dynamic_cast<Enemy *>(this)->enemy_is_hit;
        redness_f = dynamic_cast<Enemy *>(this)->redness_factor;
    }


    shader->uniform("projection", current_camera()->proj);

    if(this->type == globals::ModelType::arm || type == globals::ModelType::sword_screen || type == globals::ModelType::axe_screen) {
        shader->uniform("view", glm::mat4(1));
    }
    else{
        shader->uniform("view", current_camera()->view);
    }

    shader->uniform("model", model);
    shader->uniform("model_normal", modelNormal);
    shader->uniform("id", float(id) );//* .0001f);
    shader->uniform("enemy_is_hit",hit_uniform);
    shader->uniform("redness_factor", redness_f);
    animator.render_animation(shader);
    skelModel.Draw(shader);
}

void AnimatedModel::changeAnimation(globals::AnimationType aType, int offset) {
// TODO uncomment to debug
//#define GET_VARNAME(x) (#x)
//    if(type == globals::ModelType::zombie){
//        cout << "changing animation of " << (type) << " to " << (aType) << endl;
//    }
//#undef GET_VARNAME

    this->aType = aType;
    animator.PlayAnimation(&as[aType]);
    aTimer.begin(offset);
}

void AnimatedModel::set_speed_factor(float speed_fac) {
    animator.speedfactor = speed_fac;
}
