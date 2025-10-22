#include "cl_msg.h"
#include "ambassador.h"
#include "../model/movable_model.h"
#include "../model/player.h"
#include "../model/cloud.h"
#include "../lightning.h"
#include "../model/RemotePlayer.h"

using namespace std;

extern std::shared_ptr<Player> the_player;
int the_score;

namespace {
    //
    // HANDLE RECEIVED MESSAGE FROM SERVER
    //

    void ClAddPlayer(const msg::Msg& msg){
        GRAPRA_FROM_JSON(movableObjId,int)
        GRAPRA_FROM_JSON(startPos,glm::ivec3)

        if (!the_player) {
            GRAPRA_FROM_JSON(nextId,int)
            GRAPRA_FROM_JSON(maxId,int)
            GRAPRA_FROM_JSON(intervalMs,int)
            GRAPRA_FROM_JSON(gameTimeOffset,int)
            GRAPRA_FROM_JSON(mapSize,glm::ivec3)

            cout << "minId, maxId: " << nextId << ", " << maxId << endl;

#ifndef AMBASSADOR_NO_SLEEP
            the_ambassador->interval = intervalMs;
#endif
            the_map = make_shared<Map>(mapSize);

            MovableModel::minId = nextId;
            MovableModel::maxId = maxId;
            MovableModel::nextId = nextId;

            the_score = 0;
            the_player = make_shared<Player>(movableObjId, startPos, gameTimeOffset);
            MovableModel::movable_models[movableObjId] = the_player;

            make_camera_current(the_player->player_cam);
            return;
        }

//        TODO don't create full player object?
//        MovableModel::movable_models[movableObjId] = make_shared<Player>(movableObjId, startPos, 0);

        MovableModel::movable_models[movableObjId] = std::make_shared<RemotePlayer>(movableObjId, startPos);
    }
    void ClFreeze(const msg::Msg& msg){
        GRAPRA_FROM_JSON(frozen,int)

        the_player->freezed = frozen;
    }
    void ClUpdateInventory(const msg::Msg& msg){
        GRAPRA_FROM_JSON(blockType,globals::ModelType)
        GRAPRA_FROM_JSON(change,int)

        the_player->updateInventory(blockType, change);
    }
    void ClAddMovable(const msg::Msg& msg){
        GRAPRA_FROM_JSON(movableObjId,int)
        GRAPRA_FROM_JSON(pos,glm::ivec3)
        GRAPRA_FROM_JSON(movableObjType,globals::ModelType)

        MovableModel::movable_models[movableObjId] = AnimatedModel::newAnimatedModel(movableObjId, movableObjType, pos);
    }
    void ClUpdateAnimated(const msg::Msg& msg){
        GRAPRA_FROM_JSON(movableObjId,int)
        GRAPRA_FROM_JSON(pos,glm::vec3)

        if(pos==VEC3_INVALID_POS){
            the_score++;
            MovableModel::movable_models.erase(movableObjId);
            return;
        }

        GRAPRA_FROM_JSON(rot,glm::vec2)
        GRAPRA_FROM_JSON(animationType,globals::AnimationType)
        GRAPRA_FROM_JSON(offsetMs,int)

        auto animated = dynamic_cast<AnimatedModel *>(MovableModel::movable_models.at(movableObjId).get());

        // TODO
        if(animated->id == the_player->id){
            cout << "skipped the_player" << endl;
            return;
        }

        animated->setPos(pos);
        animated->setRot(rot);
        animated->changeAnimation(animationType, offsetMs);
    }
    void ClUpdateHp(const msg::Msg& msg){
        GRAPRA_FROM_JSON(movableObjId,int)
        GRAPRA_FROM_JSON(hpChange,float)

        if(the_player->id == movableObjId){
            the_player->hp += hpChange;

            the_player->player_hit = true;
            the_player-> hit_redness_timer.begin();
        }else{
            MovableModel::movable_models.at(movableObjId)->hp += hpChange;
        }
    }
    void ClUpdateProjectile(const msg::Msg& msg){
//        GRAPRA_FROM_JSON(movableObjId,int)
//        GRAPRA_FROM_JSON(pos,glm::vec3)
//        GRAPRA_FROM_JSON(velocity,glm::vec3)

        // TODO
    }
    void ClUpdateBlock(const msg::Msg& msg){
        GRAPRA_FROM_JSON(blockType,globals::ModelType)
        GRAPRA_FROM_JSON(pos,glm::ivec3)

        const auto block = Block::newBlock(blockType, pos);
        the_map->set(pos, block);
    }
    void ClUpdateCloud(const msg::Msg& msg){
        GRAPRA_FROM_JSON(movableObjId,int)
        GRAPRA_FROM_JSON(cloudType,globals::ModelType)
        GRAPRA_FROM_JSON(pos,glm::vec2)

        if(pos == VEC2_INVALID_POS){
            Cloud::all_clouds.erase(movableObjId);
        }else {
            Cloud::all_clouds[movableObjId] = std::make_shared<Cloud>(cloudType, pos);
        }
    }
    void ClThunderstorm(const msg::Msg& msg){
        GRAPRA_FROM_JSON(active,bool)

        if(active){
            Cloud::darken_clouds();
        }else{
            Cloud::whiten_clouds();
        }
    }
    void ClLightning(const msg::Msg& msg){
        GRAPRA_FROM_JSON(movableObjId,int)

        Lightning::spawn_lightning(movableObjId);
    }
    void ClExit(const msg::Msg& msg){
        // TODO
    }
}

//
// SEND TO SERVER
//

void msg::client::SvLogin(const std::string &playerName) {
    Msg msg(msg::MsgType::SV_LOGIN);
    GRAPRA_TO_JSON_1(playerName)
    the_ambassador->addOutgoing(msg);
}

void msg::client::SvLogout() {
    Msg msg(msg::MsgType::SV_LOGOUT);
    the_ambassador->addOutgoing(msg);
}

void msg::client::SvUpdatePlayer(glm::vec3 newPos, glm::vec2 rot, globals::AnimationType animationType, int offsetMs) {
    Msg msg(msg::MsgType::SV_UPDATE_PLAYER);
    GRAPRA_TO_JSON_4(newPos,rot,animationType,offsetMs)
    the_ambassador->addOutgoing(msg);
}

void msg::client::SvUpdateHp(int movableObjId, float hpChange) {
    Msg msg(msg::MsgType::SV_UPDATE_HP);
    GRAPRA_TO_JSON_2(movableObjId,hpChange)
    the_ambassador->addOutgoing(msg);
}

void msg::client::SvDig(globals::ModelType oldBlockType, glm::ivec3 pos) {
    Msg msg(msg::MsgType::SV_DIG);
    GRAPRA_TO_JSON_2(oldBlockType,pos)
    the_ambassador->addOutgoing(msg);
}

void msg::client::SvPlace(globals::ModelType newBlockType, glm::ivec3 pos) {
    Msg msg(msg::MsgType::SV_PLACE);
    GRAPRA_TO_JSON_2(newBlockType,pos)
    the_ambassador->addOutgoing(msg);
}

void msg::client::SvAddProjectile(int movableObjId, glm::vec3 pos, glm::vec3 velocity) {
    Msg msg(msg::MsgType::SV_ADD_PROJECTILE);
    GRAPRA_TO_JSON_3(movableObjId,pos,velocity)
    the_ambassador->addOutgoing(msg);
}

void msg::client::SvUpdateProjectile(int movableObjId, glm::vec3 finalPos) {
    Msg msg(msg::MsgType::SV_UPDATE_PROJECTILE);
    GRAPRA_TO_JSON_2(movableObjId,finalPos)
    the_ambassador->addOutgoing(msg);
}

//
// RECEIVE FROM SERVER
//

bool msg::client::handleIncoming() {
    const std::vector<Msg>& receivedMsgs = the_ambassador->takeIncoming();

    for(const auto& msg:receivedMsgs){
        // cout << "handleIncomingMsg: " << msg.json.dump() << endl; // TODO

        switch(msg.type){
            case MsgType::SV_LOGIN:
            case MsgType::SV_LOGOUT:
            case MsgType::SV_UPDATE_PLAYER:
            case MsgType::SV_UPDATE_HP:
            case MsgType::SV_DIG:
            case MsgType::SV_PLACE:
            case MsgType::SV_ADD_PROJECTILE:
            case MsgType::SV_UPDATE_PROJECTILE:
            case MsgType::MESSAGE_TYPE_ERROR:
                throw runtime_error("network error");

            case MsgType::CL_ADD_PLAYER:
                ClAddPlayer(msg);
                break;
            case MsgType::CL_FREEZE:
                ClFreeze(msg);
                break;
            case MsgType::CL_UPDATE_INVENTORY:
                ClUpdateInventory(msg);
                break;
            case MsgType::CL_ADD_MOVABLE:
                ClAddMovable(msg);
                break;
            case MsgType::CL_UPDATE_ANIMATED:
                ClUpdateAnimated(msg);
                break;
            case MsgType::CL_UPDATE_HP:
                ClUpdateHp(msg);
                break;
            case MsgType::CL_UPDATE_PROJECTILE:
                ClUpdateProjectile(msg);
                break;
            case MsgType::CL_UPDATE_BLOCK:
                ClUpdateBlock(msg);
                break;
            case MsgType::CL_UPDATE_CLOUD:
                ClUpdateCloud(msg);
                break;
            case MsgType::CL_THUNDERSTORM:
                ClThunderstorm(msg);
                break;
            case MsgType::CL_LIGHTING:
                ClLightning(msg);
                break;
            case MsgType::CL_EXIT:
                ClExit(msg);
                break;
        }
    }

    return !receivedMsgs.empty();
}
