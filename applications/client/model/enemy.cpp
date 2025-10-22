#include "enemy.h"
#include "cppgl/src/cppgl.h"
#include "Message.h"
#include "globals.h"
#include "../map/map.h"

const float ENEMY_HIT_TIME = 1000.f;


Enemy::Enemy(int id, glm::ivec3 pos, globals::ModelType type)
        : Base(id,
               type == globals::ModelType::zombie ? globals::ModelType::zombie : globals::ModelType::skeleton,
               pos,
               glm::vec2(0),
               glm::vec3(1.f))
{

}

void Enemy::update() {
    if(enemy_is_hit){
        float dt = hit_redness_timer.look();
        if(dt >= ENEMY_HIT_TIME){
            enemy_is_hit = false;
            redness_factor = 0.f;
        }
        else{
            redness_factor = 1 - (dt / ENEMY_HIT_TIME);
        }
    }

    //TODO change animation when dying
    Base::update();

}
