#include "lightning.h"
#include <cppgl.h>
#include "model/player.h"
#include "model/cloud.h"


std::vector<Drawelement> Lightning::prototype;
std::vector<Texture2D> Lightning::tex;
std::shared_ptr<Lightning> Lightning::the_lightning;
extern std::shared_ptr<Player> the_player;


void Lightning::init_lightning()
{
    Lightning::tex.emplace_back("lightning0", "render-data/lightning_bolt/0.png");
    Lightning::tex.emplace_back("lightning1", "render-data/lightning_bolt/1.png");
    Lightning::tex.emplace_back("lightning2", "render-data/lightning_bolt/2.png");
    Lightning::tex.emplace_back("lightning3", "render-data/lightning_bolt/3.png");
    Lightning::tex.emplace_back("lightning4", "render-data/lightning_bolt/4.png");
    Lightning::tex.emplace_back("lightning5", "render-data/lightning_bolt/5.png");
    Lightning::tex.emplace_back("lightning6", "render-data/lightning_bolt/6.png");
    Lightning::tex.emplace_back("lightning7", "render-data/lightning_bolt/7.png");
    Lightning::tex.emplace_back("lightning8", "render-data/lightning_bolt/8.png");
    Lightning::tex.emplace_back("lightning9", "render-data/lightning_bolt/9.png");
    Lightning::tex.emplace_back("lightning10", "render-data/lightning_bolt/10.png");
    Lightning::tex.emplace_back("lightning11", "render-data/lightning_bolt/11.png");

    glm::vec3 vertices[4] = { {-0.5,-0.5,0}, {-0.5,0.5,0}, {0.5,0.5,0}, {0.5,-0.5,0} };

    glm::vec2 texcoords[4] = { {0, 0}, {0, 1}, {1, 1}, {1, 0} };
    unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
    auto mesh = Mesh("lightning-mesh");
    mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices);

    mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texcoords);
    mesh->add_index_buffer(6, indices);

    static Shader shader { "lightning-shader", "shader/sun.vs", "shader/sun.fs" };
    Lightning::prototype.emplace_back("lightning", shader, mesh);
}

Lightning::Lightning(glm::vec2 pos) : curr_dir({1,0}), model(1), rot(glm::mat4(1)) {
    model[1][1] = 50.f;
    model[2][2] = 30.f;
    model[0][0] = 30.f;
    model[3][0] = pos.x - 4.f;
    model[3][1] = 16.5f;
    model[3][2] = pos.y;
    model = glm::rotate(model, glm::radians(90.f), glm::vec3(0,1,0));

    point_light1 = std::make_shared<PointLight>(glm::vec3(model[3][0], 35.f, model[3][2]), true);
    point_light2 = std::make_shared<PointLight>(glm::vec3(model[3][0], 25.f, model[3][2]), true);
    point_light3 = std::make_shared<PointLight>(glm::vec3(model[3][0], 15.f, model[3][2]), true);

    PointLight::point_lights.push_back(point_light1);
    PointLight::point_lights.push_back(point_light2);
    PointLight::point_lights.push_back(point_light3);
}

Lightning::~Lightning() {
    for(auto it = PointLight::point_lights.begin(); it != PointLight::point_lights.end(); ){
        if((*it) == point_light1 || (*it) == point_light2 || (*it) == point_light3){
            it = PointLight::point_lights.erase(it);
        }
        else{
            it++;
        }
    }
}

void Lightning::spawn_lightning(glm::ivec2 pos){
    if(!Lightning::the_lightning){
        the_lightning = make_shared<Lightning>(pos);
    }
}

void Lightning::spawn_lightning(int cloudId) {
    const auto pos = Cloud::all_clouds.at(cloudId)->getPos();
    spawn_lightning({pos.x, pos.z});
}

void Lightning::draw() {
    update();
    for (auto& elem : prototype) {


        glm::mat4 v = current_camera()->view;

        glm::mat4 modelview = v * model;


        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        elem->model = (model * rot) ;
        elem->bind();
        elem->shader->bind();
        elem->shader->uniform("proj", current_camera()->proj);

        elem->shader->uniform("model", elem->model ) ;
        elem->shader->uniform("view", current_camera()->view);

        elem->shader->uniform("modelview", modelview);

        const float LIGHTNING_TEXTURE_TIME = 40.f;
        if (timer.look() > LIGHTNING_TEXTURE_TIME) {
            cnt = (cnt+1) % 12;
            if(cnt == 11){
                lightning_finished = true;
                // this is the last frame of the lightning animation
                // finish this drawing cycle and then delete the lightning object
            }
            timer.begin();
        }
        elem->shader->uniform("tex", tex[cnt], 0);

        elem->draw();
        elem->unbind();
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }


}


void Lightning::update(){
    glm::vec2 dir = glm::vec2(current_camera()->dir.x, current_camera()->dir.z);

    //calc angle
    float dot_product = glm::dot(curr_dir, dir);

    float angle = glm::acos(dot_product / (length(curr_dir) * length(dir)));

    if(dir.x >= 0 && dir.y >= 0){
        angle = -angle;
    }else if(dir.x < 0 && dir.y >= 0){
        angle = -angle;
    }
    rot = glm::rotate(glm::mat4(1), angle, glm::vec3(0,1,0));
}

glm::vec3 Lightning::get_sun_pos()
{
    return glm::vec3(model[3][0], model[3][1], model[3][2]);
}


