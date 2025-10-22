#ifndef GRAPA_SV_MSG_H
#define GRAPA_SV_MSG_H

#include "Message.h"

namespace msg::server{
    //
    // CREATE MESSAGE FOR CLIENT
    //

    [[nodiscard]] msg::Msg ClAddPlayer(int movableObjId, glm::ivec3 startPos, int intervalMs, int nextId, int maxId, int gameTimeOffset, glm::ivec3 mapSize);
    [[nodiscard]] msg::Msg ClFreeze(bool frozen);
    [[nodiscard]] msg::Msg ClUpdateInventory(globals::ModelType blockType, int change);
    [[nodiscard]] msg::Msg ClAddMovable(int movableObjId, glm::ivec3 pos, globals::ModelType movableObjType);
    [[nodiscard]] msg::Msg ClRemoveNpc(int movableObjId);
    [[nodiscard]] msg::Msg ClUpdateAnimated(int movableObjId, glm::vec3 pos, glm::vec2 rot, globals::AnimationType animationType, int offsetMs );
    [[nodiscard]] msg::Msg ClUpdateHp(int movableObjId, float hpChange);
    [[nodiscard]] msg::Msg ClUpdateProjectile(int movableObjId, glm::vec3 pos, glm::vec3 velocity);
    [[nodiscard]] msg::Msg ClUpdateBlock(globals::ModelType blockType, glm::ivec3 pos);
    [[nodiscard]] msg::Msg ClUpdateCloud(int movableObjId, globals::ModelType cloudType, glm::ivec2 pos);
    [[nodiscard]] msg::Msg ClThunderstorm(bool active);
    [[nodiscard]] msg::Msg ClLighting(int movableObjId);
    [[nodiscard]] msg::Msg ClExit();
}

#endif //GRAPA_SV_MSG_H
