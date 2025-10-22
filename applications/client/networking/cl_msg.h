#ifndef GRAPA_CL_MSG_H
#define GRAPA_CL_MSG_H

#include <cppgl.h>
#include "../map/map.h"
#include "Message.h"

namespace msg::client{
    //
    // SEND TO SERVER
    //

    void SvLogin(const std::string &playerName);
    void SvLogout();
    void SvUpdatePlayer(glm::vec3 newPos, glm::vec2 rot, globals::AnimationType animationType, int offsetMs);
    void SvUpdateHp(int movableObjId, float hpChange);
    void SvDig(globals::ModelType oldBlockType, glm::ivec3 pos);
    void SvPlace(globals::ModelType newBlockType, glm::ivec3 pos);
    void SvAddProjectile(int movableObjId, glm::vec3 pos, glm::vec3 velocity);
    void SvUpdateProjectile(int movableObjId, glm::vec3 finalPos);

    //
    // RECEIVE FROM SERVER
    //

    /**
     * @return True if at least one msg was received.
     */
    bool handleIncoming();
}

#endif //GRAPA_CL_MSG_H
