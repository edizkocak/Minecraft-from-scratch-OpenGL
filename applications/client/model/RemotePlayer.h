#ifndef GRAPA_REMOTEPLAYER_H
#define GRAPA_REMOTEPLAYER_H

#include "animated_model.h"

class RemotePlayer : public AnimatedModel {
public:
    using Base = AnimatedModel;

    RemotePlayer(int id, glm::vec3 pos);
};


#endif //GRAPA_REMOTEPLAYER_H
