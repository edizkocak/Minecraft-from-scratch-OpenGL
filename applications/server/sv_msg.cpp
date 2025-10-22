#include "sv_msg.h"

msg::Msg msg::server::ClAddPlayer(int movableObjId, glm::ivec3 startPos, int intervalMs, int nextId, int maxId, int gameTimeOffset, glm::ivec3 mapSize) {
    Msg msg(msg::MsgType::CL_ADD_PLAYER);
    GRAPRA_TO_JSON_7(movableObjId,startPos,intervalMs,nextId,maxId,gameTimeOffset,mapSize)
    return msg;
}

msg::Msg msg::server::ClFreeze(bool frozen) {
    Msg msg(msg::MsgType::CL_FREEZE);
    GRAPRA_TO_JSON_1(frozen)
    return msg;
}

msg::Msg msg::server::ClUpdateInventory(globals::ModelType blockType, int change) {
    Msg msg(msg::MsgType::CL_UPDATE_INVENTORY);
    GRAPRA_TO_JSON_2(blockType,change)
    return msg;
}

msg::Msg msg::server::ClAddMovable(int movableObjId, glm::ivec3 pos, globals::ModelType movableObjType) {
    Msg msg(msg::MsgType::CL_ADD_MOVABLE);
    GRAPRA_TO_JSON_3(movableObjId,pos,movableObjType)
    return msg;
}

msg::Msg msg::server::ClRemoveNpc(int movableObjId) {
    static const auto pos = VEC3_INVALID_POS;
    Msg msg(msg::MsgType::CL_UPDATE_ANIMATED);
    GRAPRA_TO_JSON_2(movableObjId,pos)
    return msg;
}

msg::Msg msg::server::ClUpdateAnimated(int movableObjId, glm::vec3 pos, glm::vec2 rot, globals::AnimationType animationType, int offsetMs) {
    Msg msg(msg::MsgType::CL_UPDATE_ANIMATED);
    GRAPRA_TO_JSON_5(movableObjId,pos,rot,animationType,offsetMs)
    return msg;
}

msg::Msg msg::server::ClUpdateHp(int movableObjId, float hpChange) {
    Msg msg(msg::MsgType::CL_UPDATE_HP);
    GRAPRA_TO_JSON_2(movableObjId,hpChange)
    return msg;
}

msg::Msg msg::server::ClUpdateProjectile(int movableObjId, glm::vec3 pos, glm::vec3 velocity) {
    Msg msg(msg::MsgType::CL_UPDATE_PROJECTILE);
    GRAPRA_TO_JSON_3(movableObjId,pos,velocity)
    return msg;
}

msg::Msg msg::server::ClUpdateBlock(globals::ModelType blockType, glm::ivec3 pos) {
    Msg msg(msg::MsgType::CL_UPDATE_BLOCK);
    GRAPRA_TO_JSON_2(blockType,pos)
    return msg;
}

msg::Msg msg::server::ClUpdateCloud(int movableObjId, globals::ModelType cloudType, glm::ivec2 pos) {
    Msg msg(msg::MsgType::CL_UPDATE_CLOUD);
    GRAPRA_TO_JSON_3(movableObjId,cloudType,pos)
    return msg;
}

msg::Msg msg::server::ClThunderstorm(bool active) {
    Msg msg(msg::MsgType::CL_THUNDERSTORM);
    GRAPRA_TO_JSON_1(active)
    return msg;
}

msg::Msg msg::server::ClLighting(int movableObjId) {
    Msg msg(msg::MsgType::CL_LIGHTING);
    GRAPRA_TO_JSON_1(movableObjId)
    return msg;
}

msg::Msg msg::server::ClExit() {
    Msg msg(msg::MsgType::CL_EXIT);
    return msg;
}
