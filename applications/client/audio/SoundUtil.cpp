#include "SoundUtil.h"

irrklang::vec3df SoundUtil::toVec3(const glm::vec3& v) {
    return {v.x, v.y, v.z};
}
