#include "flare.h"
#include "particles.h"
#include <cppgl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "sky.h"


Drawelement Flare::prototype;
extern std::shared_ptr<Sun> the_sun;


void init_flare() {
    glm::vec3 vertices[4] = { {-0.5,-0.5,0}, {-0.5,0.5,0}, {0.5,0.5,0}, {0.5,-0.5,0} };

    glm::vec2 texcoords[4] = { {0, 0}, {0, 1}, {1, 1}, {1, 0} };
    unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
    auto mesh = Mesh("floor-mesh");
    mesh->add_vertex_buffer(GL_FLOAT, 3, 4, vertices);

    mesh->add_vertex_buffer(GL_FLOAT, 2, 4, texcoords);
    mesh->add_index_buffer(6, indices);

    auto shader = Shader("floor-shader", "shader/floor.vs", "shader/floor.fs");
    Flare::prototype = Drawelement("floor", shader, mesh);

}


Flare::Flare(Texture2D texture, float scale) :texture(texture), scale(scale), model(1) {
    model[0][0] = 1;
    model[2][2] = 1;
    model[1][1] = 1;
    glm::vec2 vertices[4] = { {-0.5f,-0.5f}, {-0.5f,0.5f}, {0.5f,-0.5f}, {0.5f,0.5f} };

}


Flare::Flare() :model(1) {
    model[0][0] = 0.5;
    model[2][2] = 0.5;
    model[1][1] = 0.5;
}

void Flare::draw() {
    glDisable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    prototype->model = model;
    prototype->bind();
    prototype->shader->uniform("brightness", 0.8f);
    //
    //
    float xScale = scale;
    const glm::ivec2 screen_size = Context::resolution();
    float yScale = xScale * (float)screen_size.x / screen_size.y;
    glm::vec2 centerPos = screenPos;
    prototype->shader->uniform("transform", glm::vec4(centerPos.x, centerPos.y, xScale, yScale));
    prototype->shader->uniform("flareTexture", texture, 1);

   // prototype->shader->uniform("transform", glm::vec4(centerPos.x, centerPos.y, xScale, yScale));
  //  prototype->shader->uniform("brightness", 0.8f);
    prototype->draw();
    prototype->unbind();
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

}


FlareHelper::FlareHelper(float spacing) :spacing(spacing) {
  
   flares.push_back(std::make_shared<Flare>(Texture2D("0", "render-data/lensflare/ltex7.png"), 0.04));
   flares.push_back(std::make_shared<Flare>(Texture2D("1", "render-data/lensflare/ltex0.png"), 0.04));
   // flares.push_back(std::make_shared<Flare>(Texture2D("2", "render-data/lensflare/ltex5.png"), 0.2));
    flares.push_back(std::make_shared<Flare>(Texture2D("9", "render-data/lensflare/ltex6.png"), 0.05));
    flares.push_back(std::make_shared<Flare>(Texture2D("12", "render-data/lensflare/ltex4.png"), 0.08));
   
    flares.push_back(std::make_shared<Flare>(Texture2D("4", "render-data/lensflare/ltex1.png"), 0.04));
    flares.push_back(std::make_shared<Flare>(Texture2D("5", "render-data/lensflare/ltex9.png"), 0.13));
   flares.push_back(std::make_shared<Flare>(Texture2D("3", "render-data/lensflare/ltex4.png"), 0.07));
   flares.push_back(std::make_shared<Flare>(Texture2D("10", "render-data/lensflare/ltex2.png"), 0.1));
   
  //  flares.push_back(std::make_shared<Flare>(Texture2D("6", "render-data/images/tex8.png"), 0.3));

}

void FlareHelper::render(glm::vec3 sunWorldPos) {
    auto camera = current_camera();
    
    sunWorldPos = the_sun->get_sun_pos();
    if(sunWorldPos.y < 0){return;}
    glm::vec2 sunCoords = convertToScreenSpace(sunWorldPos, camera->view, camera->proj);
    if (sunCoords == glm::vec2(-5.f, -5.f)) {
        return;
    }

    glm::vec2 sunToCenter = CENTER_SCREEN - sunCoords;
    float l = (sqrt(sunToCenter.x * sunToCenter.x + sunToCenter.y * sunToCenter.y));
    float brightness = 1 - ((sqrt(sunToCenter.x * sunToCenter.x + sunToCenter.y * sunToCenter.y)) / 0.7f);
   // std::cout << brightness << std::endl;
    if (brightness > 0) {
        calcFlarePositions(sunToCenter, sunCoords);
        //renderer.render(flareTextures, brightness);
        for (unsigned long int i = 0; i < flares.size(); i++) {
            flares[i]->draw();
        }
        
    }
}

void FlareHelper::calcFlarePositions(glm::vec2 sunToCenter, glm::vec2 sunCoords) {
   for (unsigned long int i = 0; i < flares.size(); i++) {
        glm::vec2 direction = glm::vec2(sunToCenter.x,sunToCenter.y);
   
        direction *= ((float)i * spacing);
        glm::vec2 flarePos = sunCoords + direction;
        flares[i]->screenPos = flarePos;
    }
}

glm::vec2 FlareHelper::convertToScreenSpace(glm::vec3 worldPos, glm::mat4 viewMat, glm::mat4 projectionMat) {
    glm::vec4 screen_c = glm::vec4(worldPos.x, worldPos.y, worldPos.z, 1.f);
    screen_c = viewMat * screen_c;
    screen_c = projectionMat * screen_c;

    if (screen_c.w <= 0) {
        return glm::vec2(-5.f, -5.f);
    }

    float x = (screen_c.x / screen_c.w + 1) / 2.f;
    float y = 1 - ((screen_c.y / screen_c.w + 1) / 2.f);
    return glm::vec2(x, y);
}

