#include "point_light.h"
#include "rendering/pass.h"
#include "rendering/buffer.h"
#include <cppgl.h>
#include "map/map.h"
#include "model/movable_model.h"
#include "model/animated_model.h"
#include "particles.h"

extern std::shared_ptr<Map> the_map;
extern std::shared_ptr<Particles> particles_torch;

std::vector<std::shared_ptr<PointLight>> PointLight::point_lights;
Camera PointLight::pShadow_cam = Camera("pShadow_cam");
glm::mat4 PointLight::shadowMatrices[6];


PointLight::PointLight(glm::vec3 pos, bool lightning) : pos(pos), lightning(lightning){
    glCreateFramebuffers(1, &pShadow_buffer);
    //texture for depth buffer
    glGenTextures(1, &cube_depth);

    const unsigned int SHADOW_WIDTH = 256, SHADOW_HEIGHT = 256;
    const unsigned int LIGHTNING_SHADOW_WIDTH = 1024, LIGHTNING_SHADOW_HEIGHT = 1024;
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube_depth);

    if(lightning){
        for (unsigned int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                         LIGHTNING_SHADOW_WIDTH, LIGHTNING_SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        }
    }
    else {
        for (unsigned int i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                         SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    float color[4]= {1.f, 1.f, 1.f, 1.f};
    glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, color);

    glBindFramebuffer(GL_FRAMEBUFFER, pShadow_buffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cube_depth, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Framebuffer not complete: " << fboStatus << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    ambient = lightning ? glm::vec3(5.f) : glm::vec3(2.f, 2.f, 2.f);
    diffuse = lightning ? glm::vec3(1.f, 1.f, 1.f) : glm::vec3(212.f / 255.f, 71.f / 255.f, 11.f / 255.f);
    specular =  glm::vec3(1.0f, 1.0f, 1.0f);
}



void PointLight::camera_face(int faceIndex){
    switch (faceIndex) {
        case 0:
            pShadow_cam->dir = glm::vec3(1.0, 0.0, 0.0);
            pShadow_cam->up = glm::vec3(0.0, -1.0, 0.0);
            break;
        case 1:
            pShadow_cam->dir = glm::vec3(-1.0, 0.0, 0.0);
            pShadow_cam->up = glm::vec3(0.0, -1.0, 0.0);
            break;
        case 2:
            pShadow_cam->dir = glm::vec3(0.0, 1.0, 0.0);
            pShadow_cam->up = glm::vec3(0.0, 0.0, 1.0);
            break;
        case 3:
            pShadow_cam->dir = glm::vec3(0.0, -1.0, 0.0);
            pShadow_cam->up = glm::vec3(0.0, 0.0, -1.0);
            break;
        case 4:
            pShadow_cam->dir = glm::vec3(0.0, 0.0, 1.0);
            pShadow_cam->up = glm::vec3(0.0, -1.0, 0.0);
            break;
        case 5:
            pShadow_cam->dir = glm::vec3(0.0, 0.0, -1.0);
            pShadow_cam->up = glm::vec3(0.0, -1.0, 0.0);
            break;
    }
}


void PointLight::draw_point_shadows(){
    static Shader pShadow_shader { "pShadow_shader", "shader/point_shadow.vs", "shader/point_shadow.gs", "shader/point_shadow.fs" };

    using namespace rendering::buffer;

    // draw the cubemap only every 3 frames
    if(frame_count == 0){
        frame_count = (frame_count + 1) % 3;
    }
    else{
        frame_count = (frame_count + 1) % 3;
        return;
    }

    if(lightning){
        glViewport(0, 0, 1024, 1024);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, pShadow_buffer);

    glClear(GL_DEPTH_BUFFER_BIT);

    // set camera
    pShadow_cam->near = 0.01f;
    pShadow_cam->far = lightning ? 200.f : 6.f;
    pShadow_cam->pos = pos + glm::vec3(0,0.05f,0);
    for (int i = 0; i < 6; i++) {
        camera_face(i);
        pShadow_cam->fov_degree = 90.f;
        pShadow_cam->update();
        shadowMatrices[i] = pShadow_cam->proj * pShadow_cam->view;
    }

    pShadow_shader->bind();

    pShadow_shader->uniform("lightPos", pShadow_cam->pos);
    pShadow_shader->uniform("far_plane", pShadow_cam->far);
    pShadow_shader->uniform("animated_model", false);


    for (int i = 0; i < 6; ++i) {
        pShadow_shader->uniform("shadowMatrices[" + std::to_string(i) + "]", shadowMatrices[i]);
    }


    // TODO: dont draw the torch itself
    // TODO: adjust shader to support animated models
    the_map->draw(pShadow_shader);
    for (auto &[id, model] : MovableModel::movable_models) {
        //if(dynamic_cast<AnimatedModel *>(model.get())) continue;
        pShadow_shader->uniform("animated_model", true);

        model->draw(pShadow_shader);
    }
    pShadow_shader->unbind();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // add particles
    if(!lightning){
        const glm::vec3 dir = glm::normalize(glm::vec3(random_float(), 0.1f, random_float()));
        //particles_torch->add(pos, dir, glm::vec4(212.f / 255.f, 71.f / 255.f, 11.f / 255.f, 1.f), (rand() % 1000) + 1000);
    }
}
