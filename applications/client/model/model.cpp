#include "model.h"
#include "player.h"
#include "cloud.h"
#include "../particles.h"

using namespace std;
namespace filesystem = fs;

extern std::shared_ptr<Player> the_player;
extern std::shared_ptr<Particles> particles_torch;
extern bool shadow_pass;

std::map<globals::ModelType, std::vector<Mesh>> Model::meshes;

Model::Model(globals::ModelType type,
             glm::vec3 pos,
             glm::vec2 rot,
             glm::vec3 scale)
        : type(type) {

    if (meshes[type].empty()) {
        load();
    }

    setScale(scale);
    setPos(pos);
    setRot(glm::vec3(rot,0));

    doUpdateObj();
}

void Model::draw(Shader &shader) {
    if(!visible) return;

    bool is_cloud = false;
    if(dynamic_cast<Cloud *>(this)){
        is_cloud = true;
    }

    shader->uniform("model", model);
    shader->uniform("model_normal", modelNormal);
    shader->uniform("highlighted", highlighted);
    shader->uniform("mining_progress", the_player->mining_progress);
    shader->uniform("is_cloud", is_cloud);

    for (auto &mesh: meshes[type]) {
        mesh->bind(shader);
        mesh->draw();
        mesh->unbind();
    }

    if(!shadow_pass) {
        highlighted = false; // TODO what if draw() is called twice (during one main render-loop) with different shaders?
    }

    if(type == globals::ModelType::torch){
          particles_torch->add_torch_particles(glm::vec3(model[3][0],model[3][1],model[3][2]) );
           particles_torch->set_torch();
      
    }


}

void Model::update() {
    doUpdateObj();
}

void Model::updateVisibility() {
    visible = glm::distance(_pos, the_player->getPos()) <= globals::viewDistance;
}

void Model::doUpdateObj() {
    update(getUp());
}

void Model::update(const glm::vec3 up) {
    const glm::vec3 scaledSide = _side * _scale.x;
    const glm::vec3 scaledUp = up * _scale.y;
    const glm::vec3 scaledDir = -1.f * _dir * _scale.z; // TODO

    // model = glm::mat4(1);

    model[0][0] = scaledSide[0];
    model[0][1] = scaledSide[1];
    model[0][2] = scaledSide[2];

    model[1][0] = scaledUp[0];
    model[1][1] = scaledUp[1];
    model[1][2] = scaledUp[2];

    model[2][0] = scaledDir[0];
    model[2][1] = scaledDir[1];
    model[2][2] = scaledDir[2];

    model[3][0] = _pos[0];
    model[3][1] = _pos[1];
    model[3][2] = _pos[2];

    modelNormal = glm::transpose(glm::inverse(model));

   
}

void Model::load() {
    fs::path file;

    switch(type){
        case globals::ModelType::water:
            file = globals::renderData() / "Cubes" / "water" / "water.obj";
            break;
        case globals::ModelType::wood:
            file = globals::renderData() / "Cubes" / "wood" / "wood.obj";
            break;
        case globals::ModelType::dirt:
            file = globals::renderData() / "Cubes" / "dirt" / "dirt.obj";
            break;
        case globals::ModelType::grass:
            file = globals::renderData() / "Cubes" / "grass" / "grass.obj";
            break;
        case globals::ModelType::leaf:
            file = globals::renderData() / "Cubes" / "leafs" / "leafs.obj";
            break;
        case globals::ModelType::stone:
            file = globals::renderData() / "Cubes" / "stone" / "stone.obj";
            break;
        case globals::ModelType::torch:
            file = globals::renderData() / "Cubes" / "torch" / "torch.obj";
            break;
        case globals::ModelType::cloud0:
            file = globals::renderData() / "Cubes" / "clouds" / "cloud0.obj";
            break;
        case globals::ModelType::cloud1:
            file = globals::renderData() / "Cubes" / "clouds" / "cloud1.obj";
            break;
        case globals::ModelType::cloud2:
            file = globals::renderData() / "Cubes" / "clouds" / "cloud2.obj";
            break;
        case globals::ModelType::cloud3:
            file = globals::renderData() / "Cubes" / "clouds" / "cloud3.obj";
            break;
        case globals::ModelType::axe_screen:
            file = globals::renderData() / "on_screen_objects" / "axe.obj";
            break;

        case globals::ModelType::air:
            throw std::runtime_error("tried to load air");

        case globals::ModelType::player:
        case globals::ModelType::dog:
        case globals::ModelType::horse:
        case globals::ModelType::zombie:
        case globals::ModelType::skeleton:
        case globals::ModelType::arm:
        case globals::ModelType::sword:
        case globals::ModelType::axe:
        case globals::ModelType::sword_screen:

            return;
        default:
            std::cout << int(type) << std::endl;
    }

    meshes[type] = load_meshes_gpu(file, false);
}
