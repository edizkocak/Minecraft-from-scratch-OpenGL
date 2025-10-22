#include "billboard.h"
#include <cppgl.h>
#include "rendering/util.h"


std::vector<Drawelement> Billboard::prototype;
void init_textures() {

}

Billboard::Billboard(): model(1) {
    glm::vec3 vertices[4] = { {-0.5,-0.5,0}, {-0.5,0.5,0}, {0.5,0.5,0}, {0.5,-0.5,0} };

    glm::vec2 texcoords[4] = { {0, 0}, {0, 1}, {1, 1}, {1, 0} };
    unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
    auto mesh = Mesh("billboard-mesh");
    mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices);

    mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texcoords);
    mesh->add_index_buffer(6, indices);

    auto shader = Shader("sun-shader", "shader/sun.vs", "shader/sun.fs");
    Billboard::prototype.push_back( Drawelement("billboard", shader, mesh));

    model[1][1] = 100;
    model[2][2] = 100;
    model[0][0] = 100;
    model[3][0] = 20;
    model[3][1] = 50;
    model[3][2] = 10;

    timer.begin();
    cnt = 0;
}

Billboard::~Billboard() {}

void Billboard::draw() {
    for (auto& elem : prototype) {
        model[0][1] = 0;
        model[0][2] = 0;
        model[1][0] = 0;
        model[1][2] = 0;
        model[2][0] = 0;
        model[2][1] = 0;

        glm::mat4 v = current_camera()->view;
        v[0][1] = 0;
        v[0][2] = 0;
        v[1][0] = 0;
        v[1][2] = 0;
        v[2][0] = 0;
        v[2][1] = 0;
        v[1][1] = 1;
        v[2][2] = 1;
        v[0][0] = 1;
        glm::mat4 modelview = v * model;
        modelview[0][1] = 0;
        modelview[0][2] = 0;
        modelview[1][0] = 0;
        modelview[1][2] = 0;
        modelview[2][0] = 0;
        modelview[2][1] = 0;

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

      //  glm::mat4 modelview = current_camera()->view * model;
        elem->model = model;
        elem->bind();
        // TODO // rendering::util::setup_light(elem->shader);

        elem->shader->uniform("modelview", modelview);

      // float t = fmod( timer.look(), 200);
      //  if (timer.look() > 50) {
            cnt = (int)(cnt + 1 ) % 12;
        //    timer.begin();
     //  }

        elem->shader->uniform("tex", tex[cnt], 0);

        elem->draw();
        elem->unbind();
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }
}

glm::vec3 Billboard::get_sun_pos()
{
    return glm::vec3(model[3][0], model[3][1], model[3][2]);
}

void Billboard::update()
{
    float radius = 20;

}

void Billboard::load_textures()
{
    tex.emplace_back("lightning0", "render-data/lightning_bolt/0.png");
    tex.emplace_back("lightning1", "render-data/lightning_bolt/1.png");
    tex.emplace_back("lightning2", "render-data/lightning_bolt/2.png");
    tex.emplace_back("lightning3", "render-data/lightning_bolt/3.png");
    tex.emplace_back("lightning4", "render-data/lightning_bolt/4.png");
    tex.emplace_back("lightning5", "render-data/lightning_bolt/5.png");
    tex.emplace_back("lightning6", "render-data/lightning_bolt/6.png");
    tex.emplace_back("lightning7", "render-data/lightning_bolt/7.png");
    tex.emplace_back("lightning8", "render-data/lightning_bolt/8.png");
    tex.emplace_back("lightning9", "render-data/lightning_bolt/9.png");
    tex.emplace_back("lightning10", "render-data/lightning_bolt/10.png");
    tex.emplace_back("lightning11", "render-data/lightning_bolt/11.png");

}
Grass::Grass() {
    glm::vec3 vertices[4] = { {-0.5,-0.5,0}, {-0.5,0.5,0}, {0.5,0.5,0}, {0.5,-0.5,0} };

    glm::vec2 texcoords[4] = { {0, 0}, {0, 1}, {1, 1}, {1, 0} };
    unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
    auto mesh = Mesh("billboard-mesh");
    mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices);

    mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texcoords);
    mesh->add_index_buffer(6, indices);

    auto shader = Shader("sun-shader", "shader/sun.vs", "shader/sun.fs");
    Billboard::prototype.push_back(Drawelement("billboard", shader, mesh));
}


