#include "RemotePlayer.h"

RemotePlayer::RemotePlayer(int id, glm::vec3 pos)
    : Base(id, globals::ModelType::player,  pos + glm::vec3(0.5f, 0, 0.5f)){
}
