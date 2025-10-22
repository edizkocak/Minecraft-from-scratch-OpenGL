#ifndef GRAPA_SOUNDUTIL_H
#define GRAPA_SOUNDUTIL_H

#include "glm/vec3.hpp"
#include "ik_vec3d.h"

class SoundUtil {
public:
    static irrklang::vec3df toVec3(const glm::vec3& v);
};

#endif //GRAPA_SOUNDUTIL_H
