#include "animal.h"
#include <cppgl.h>
#include "globals.h"

Animal::Animal(int id, glm::ivec3 pos, globals::ModelType type)
        : Base(id,
               type == globals::ModelType::dog ? globals::ModelType::dog : globals::ModelType::horse,
               pos,
               glm::vec2(0),
               glm::vec3(1.f))
{}

void Animal::update() {
    Base::update();
}