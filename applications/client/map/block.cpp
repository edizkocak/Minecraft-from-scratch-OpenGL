#include "block.h"

using namespace std;
namespace fs = std::filesystem;

Block::Block(globals::ModelType type, glm::ivec3 pos)
        : Base(type, pos, glm::vec2{0}, glm::vec3(0.5)) {
}

std::shared_ptr<Block> Block::newBlock(globals::ModelType type, glm::ivec3 pos) {
    switch(type){
        case globals::ModelType::water:
            return make_shared<WaterBlock>(pos);
        case globals::ModelType::wood:
            return make_shared<WoodBlock>(pos);
        case globals::ModelType::dirt:
            return make_shared<DirtBlock>(pos);
        case globals::ModelType::grass:
            return make_shared<GrassBlock>(pos);
        case globals::ModelType::leaf:
            return make_shared<LeafBlock>(pos);
        case globals::ModelType::stone:
            return make_shared<StoneBlock>(pos);
        case globals::ModelType::torch:
            return make_shared<TorchBlock>(pos);

        case globals::ModelType::air:
            return nullptr;

        case globals::ModelType::player:
        case globals::ModelType::dog:
        case globals::ModelType::horse:
        case globals::ModelType::zombie:
        case globals::ModelType::skeleton:
        case globals::ModelType::cloud0:
        case globals::ModelType::cloud1:
        case globals::ModelType::cloud2:
        case globals::ModelType::cloud3:
        case globals::ModelType::sword:
        case globals::ModelType::axe:
        case globals::ModelType::arm:
        case globals::ModelType::sword_screen:
        case globals::ModelType::axe_screen:
            break;
    }
    throw invalid_argument("not a block type");
}

DirtBlock::DirtBlock(glm::ivec3 pos)
        : Base(globals::ModelType::dirt, pos) {
}

GrassBlock::GrassBlock(glm::ivec3 pos)
        : Base( globals::ModelType::grass, pos) {
}

LeafBlock::LeafBlock(glm::ivec3 pos)
        : Base(globals::ModelType::leaf, pos) {
}

StoneBlock::StoneBlock(glm::ivec3 pos)
        : Base(globals::ModelType::stone, pos) {
}

WaterBlock::WaterBlock(glm::ivec3 pos)
        : Base(globals::ModelType::water, pos) {
}

WoodBlock::WoodBlock(glm::ivec3 pos)
        : Base(globals::ModelType::wood, pos) {
}

TorchBlock::TorchBlock(glm::ivec3 pos)
        : Base(globals::ModelType::torch, pos) {
    glm::vec3 light_pos = glm::vec3(pos.x, pos.y, pos.z) + glm::vec3(0.5,1,0.5);
    point_light = make_shared<PointLight>(light_pos, false);
    PointLight::point_lights.push_back(point_light);
}

TorchBlock::~TorchBlock(){
    for(auto it = PointLight::point_lights.begin(); it != PointLight::point_lights.end(); it++){
        if((*it) == point_light){
            PointLight::point_lights.erase(it);
            break;
        }
    }
}
