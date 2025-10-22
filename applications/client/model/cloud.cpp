#include "cloud.h"
#include <cppgl.h>
#include "../rain.h"
#include "../../common/networking/MathUtil.h"
#include "../map/map.h"
#include "player.h"

extern std::shared_ptr<Player> the_player;
extern std::shared_ptr<Rain> particles_rain;
extern glm::ivec3 map_size;
extern bool rain_on;

namespace {
    const float max_oob_dist = 20.f;   // max distance a cloud can be out of bounds
    const float CLOUD_SPEED = 0.0005f;
}

Cloud::Cloud(globals::ModelType type, glm::ivec2 pos)
: Base(type, {pos.x, 40, pos.y}){
}

void Cloud::darken_clouds() {
    cloud_darkening = true;
    darkness_timer.begin();
}

void Cloud::whiten_clouds() {
    cloud_whitening = true;
    darkness_timer.begin();
    rain_on = false;
}

void Cloud::rain_around_player(glm::vec3 pos){
    const double RAIN_RADIUS = 25;

    for(int i = 0; i < 10000; i++) {
        const auto r = float(RAIN_RADIUS * sqrt(MathUtil::rand_0_1()));
        const auto theta = float(MathUtil::rand_0_1() * 2 * M_PI);
        particles_rain->add(
                glm::vec3(
                        pos.x + r * cos(theta),
                        50 + MathUtil::rand_0_1() * 60 ,
                        pos.z + r * sin(theta)

                ),
                glm::vec3(0, -1, 0),
                glm::vec4(0.0275,0.451,0.4784,1) * 0.5f,
                100 // ignored
        );
    }
}

void Cloud::updateDarkness() {
    if (cloud_whitening) {
        cloud_darkness_factor = (float) darkness_timer.look() / globals::CLOUD_CHANGE_TIME;
        cloud_darkness_factor = 1.f - cloud_darkness_factor;
        if (cloud_darkness_factor <= 0.f) {
            cloud_darkness_factor = 0.f;
            cloud_whitening = false;
        }
    } else if (cloud_darkening) {
        cloud_darkness_factor = (float) darkness_timer.look() / globals::CLOUD_CHANGE_TIME;
        if (cloud_darkness_factor >= 1.f) {
            cloud_darkness_factor = 1.f;
            cloud_darkening = false;
            rain_on = true;
        }
    }
}

void Cloud::update(){
    auto dt_ms = float(Context::frame_time());
    _pos += dir * CLOUD_SPEED * dt_ms;
    if(_pos.x > (float) the_map->extent.x + max_oob_dist){
        _pos.x = -max_oob_dist;
    }
    if(_pos.x > (float) the_map->extent.x + max_oob_dist){
        _pos.x = -max_oob_dist;
    }
    if(_pos.z > (float) the_map->extent.z + max_oob_dist){
        _pos.z = -max_oob_dist;
    }
    if(_pos.z > (float) the_map->extent.z + max_oob_dist){
        _pos.z = -max_oob_dist;
    }

    Base::update();
}

void Cloud::updateVisibility() {
    const auto& playerPos = the_player->getPos();

    // calculate xz-distance
    float distance = glm::distance(glm::vec2(_pos.x, _pos.y), glm::vec2(playerPos.x, playerPos.z));
    // draw clouds even if little further away
    distance -= max_oob_dist;

    visible = distance <= globals::viewDistance;
}

void Cloud::draw(Shader& shader){
    if(!visible) return;
    shader->uniform("cloud_darkness_factor", cloud_darkness_factor);
    Base::draw(shader);
}
