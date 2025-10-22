#include "item.h"
#include <cppgl.h>
#include "player.h"

extern std::shared_ptr<Player> the_player;


Item::Item(int id, glm::ivec3 pos, globals::ModelType type)
        : Base(id,
               type == globals::ModelType::sword ? globals::ModelType::sword : globals::ModelType::axe,
               pos,
               glm::vec2(0),
               glm::vec3(100.f))
{
}

void Item::update() {

    this->setPos(the_player->getPos());

     this->setSide(the_player->getSide());

     this->setDir(the_player->getDir());

    Base::update();
}

