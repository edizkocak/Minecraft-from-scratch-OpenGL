#include "SvMovable.h"
#include "Server.h"
#include "sv_msg.h"
#include "MathUtil.h"
#include "AStar.h"
#include <glm/gtx/spline.hpp>

using std::cout;
using std::endl;

namespace {
    SvMap* map;

    /**
     * https://stackoverflow.com/a/37331351
     */
    glm::vec3 catmull_rom_spline(const std::deque<glm::ivec3>& cp, float t) {
        const auto t_floor = (int) t;
        const auto cp_size = (int) cp.size();

        // indices of the relevant control points
        int i0 = glm::clamp<int>(t_floor - 1, 0, cp_size - 1);
        int i1 = glm::clamp<int>(t_floor, 0, cp_size - 1);
        int i2 = glm::clamp<int>(t_floor + 1, 0, cp_size - 1);
        int i3 = glm::clamp<int>(t_floor + 2, 0, cp_size - 1);

        // parameter on the local curve interval
        float local_t = glm::fract(t);

        return glm::catmullRom<glm::vec3>(cp[i0], cp[i1],
                                          cp[i2], cp[i3],
                                           local_t);
    }
}

void SvMovable::init(SvMap* _map) {
    map = _map;
}

void SvNpc::update(double ms, int i) {
    move(ms);

    static const double ATTACK_COOLDOWN = 1000;
    const auto target = SvPlayer::closestPlayer(pos);
    if(glm::distance(pos, target->pos) < 1.7f
       && ms - lastAttackMs >= ATTACK_COOLDOWN){
        // attack target
        lastAttackMs = ms;
        Server::addMsgTo(msg::server::ClUpdateHp(target->id, -15.f), target->i);
    }

    Server::addMsgTo(msg::server::ClUpdateAnimated(id, pos, {yaw, 0}, animationType,(int) ms - animationStartMs), i);
}

void SvNpc::move(double ms) {
    static const float VELOCITY = 1.6 * 1000; // 1.5 blocks per second
    static const float EPSILON = 0.33f;

    if(path.empty()) return;
    // @precond: path contains at least two entries.

    const auto pathDtMs = ms - pathStartMs;
    auto pathIdx = float(pathDtMs / VELOCITY);
    pathIdx = glm::min<float>(pathIdx, (float) path.size() - 1);

    auto prevFloatIdx = glm::max<float>(0.f, pathIdx - EPSILON);
    auto nextFloatIdx = glm::min<float>((float) path.size() - 1.f, pathIdx + EPSILON);

    const auto prevPos = catmull_rom_spline(path, prevFloatIdx);
    pos = catmull_rom_spline(path, pathIdx);
    const auto nextPos = catmull_rom_spline(path, nextFloatIdx);

    const auto posDelta = nextPos - prevPos;
    yaw = (float) -atan2(posDelta.z, posDelta.x);

    // move to center of block
    pos += glm::vec3{0.5, 0, 0.5};

//    bool moved = pathIdx == float(path.size() - 1);
//    if(moved){
//        if(animationType!=globals::AnimationType::WALKING){
//            animationType = globals::AnimationType::WALKING;
//            animationStartMs = (int) ms;
//        }
//    }else{
//        if(animationType!=globals::AnimationType::STANDING){
//            animationType = globals::AnimationType::STANDING;
//            animationStartMs = (int) ms;
//        }
//    }
}

void SvNpc::updatePath(double ms) {
    static const double RECALC_MS = 10 * 1000;
    static const float ENEMY_HEIGHT = 1.8f;
    static const float ENEMY_VIEW_RADIUS = 20.f;

    if(ms - pathStartMs > RECALC_MS){
        const glm::ivec3 start = pos;
        glm::ivec3 goal;

        const auto target = SvPlayer::closestPlayer(pos);
        if(glm::distance(pos, target->pos) > ENEMY_VIEW_RADIUS){
            goal = map->randSurfacePos(start, ENEMY_VIEW_RADIUS) + IVEC_UP;
        }else{
            // if player jumps -> use first solid block below it
            goal = map->surfaceAt(target->pos, (int) target->pos.y) + IVEC_UP;
        }

        if(start == goal) {
            path = {};
            pathStartMs = ms;
            return;
        }

        path = AStar::search(start, goal, ENEMY_HEIGHT);
        pathStartMs = ms;
    }
}

//void SvNpc::update(double ms, double dt_ms, int i) {
//    // TODO use speed and gameTime
//
//    static const float MAX_YAW = 0.5f;
//    bool moved = false;
//
//    const auto target = SvPlayer::closestPlayer(pos);
//
//    // if distance to target > 0
//    if (target->pos != pos) {
//        glm::vec3 targetDir = glm::normalize(target->pos - pos);
//        float targetYaw = MathUtil::yawFromDir(targetDir);
//
//        float delta = targetYaw - yaw;
//        delta = glm::mod(delta, float(2*M_PI));
//        if(delta > M_PI){
//            delta -= float(2*M_PI);
//        }
//
//        delta = glm::clamp(delta, -MAX_YAW, MAX_YAW);
//        yaw += delta;
//
//        // first rotate towards player, then move
//        if(glm::abs(delta) < MAX_YAW){
//            glm::vec3 yawed = glm::normalize(glm::rotate(
//                    VEC_FORWARD, yaw, VEC_UP
//            ));
//            float distance = glm::distance(target->pos, pos);
//            if(distance>0.1f){
//                float stepSize = glm::clamp(distance, 0.f, 1.f);
//                pos += yawed * stepSize;
//                moved = true;
//            }
//        }
//    }
//
//    if(moved){
//        if(animationType!=globals::AnimationType::WALKING){
//            animationType = globals::AnimationType::WALKING;
//            animationStartMs = (int) ms;
//        }
//    }else{
//        if(animationType!=globals::AnimationType::STANDING){
//            animationType = globals::AnimationType::STANDING;
//            animationStartMs = (int) ms;
//        }
//    }
//
//    Server::addMsgTo(msg::server::ClUpdateAnimated(id, pos, {yaw, 0}, animationType,(int) ms - animationStartMs), i);
//}

void SvPlayer::update() {
    if (!frozen) return;

    // check if this player can be unfrozen

    for(const auto& other:playerList){
        if (other->id == id) continue;

        if (glm::distance(other->pos, pos) < 2.f) {
            // unfreeze this player
            frozen = false;
            Server::addMsgTo(msg::server::ClFreeze(false), i);
        }
    }
}

std::shared_ptr<SvPlayer> SvPlayer::closestPlayer(glm::vec3 pos) {
    float minDist = std::numeric_limits<float>::infinity();
    auto &closest = SvPlayer::playerList[0];

    for (int i = 1; i < numPlayers; i++) {
        const auto &other = playerList[i];
        const auto dist = glm::distance(pos, other->pos);
        if (dist < minDist) {
            minDist = dist;
            closest = other;
        }
    }

    return closest;
}
