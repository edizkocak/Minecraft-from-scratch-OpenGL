#include "pass.h"

#include <cppgl.h>
#include <stdexcept>
#include <utility>
#include <glm/gtc/type_ptr.hpp>

#include "buffer.h"
#include "util.h"
#include "../map/map.h"
#include "../model/player.h"
#include "../particles.h"
#include "../model/cloud.h"
#include "../rain.h"
#include "../point_light.h"
#include "../lightning.h"
#include "../model/screen_model.h"
#include "../model/screen_model_test.h"
#include "../flare.h"
#include "../sky.h"


using namespace rendering;

extern std::shared_ptr<Sky> the_sky;
extern std::shared_ptr<Sun> the_sun;
extern std::shared_ptr<Player> the_player;
extern std::shared_ptr<FlareHelper> the_flare;

extern std::shared_ptr<Rain> particles_rain;
extern std::shared_ptr<Particles> particles_small;
extern std::shared_ptr<Particles> particles_torch;

extern std::shared_ptr<Arm> the_arm;
extern std::shared_ptr<Sword> the_sword;
extern std::shared_ptr<Axe> the_axe;

extern bool rain_on;
extern bool shadow_pass;

int MAX_POINT_LIGHTS;
//Camera pShadow_cam = Camera("point_shadow_cam");
//glm::mat4 shadowMatrices[6];


// debug
float screen_hit_radius = .3f;
float max_hit_dist = 1.f;
bool rayleigh_scattering_on = false;

float bias = 0.301f;
float far_plane = 25.f;
float off = 1.f;

float amb_col = 4.5f;


void pass::draw_pass_gui() {
    if(ImGui::CollapsingHeader("Pass GUI")){
        ImGui::SliderFloat("screen_hit_radius", &screen_hit_radius, 0.f , 2.f);
        ImGui::SliderFloat("max hit dist", &max_hit_dist, 0.f , 2.f);
        ImGui::Checkbox("rayleigh-scattering on", &rayleigh_scattering_on);
        if(ImGui::CollapsingHeader("Point Shadows")){
            ImGui::SliderFloat("bias", &bias, 0.f , 2.f);
            ImGui::SliderFloat("far_plane", &far_plane, 0.f , 25.f);
            ImGui::SliderFloat("off", &off, -10.f , 10.f);
        }
        if(ImGui::CollapsingHeader("Point Lights")){
            ImGui::SliderFloat("amb_col", &amb_col, 0.f , 5.f);
        }
    }
}


void pass::geometry() {
    static Shader pos_norm_tc { "pos_norm_tc", "shader/pos+norm+tc.vs", "shader/pos+norm+tc.fs" };

    rendering::buffer::geom->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pos_norm_tc->bind();
    util::setup_camera(pos_norm_tc, current_camera());
    pos_norm_tc->uniform("view", current_camera()->view);
    pos_norm_tc->uniform("view_normal", current_camera()->view_normal);
    pos_norm_tc->uniform("proj", current_camera()->proj);

    the_map->draw(pos_norm_tc);
    for (auto &[id, cloud] : Cloud::all_clouds) {
        cloud->draw(pos_norm_tc);
    }

    for (auto &[id, model] : MovableModel::movable_models) {
        if(dynamic_cast<AnimatedModel *>(model.get())) continue;

        model->draw(pos_norm_tc);
    }

    //the_arm->draw(pos_norm_tc);

    pos_norm_tc->unbind();
          
    the_sky->draw();
    the_sun->draw();
    if(!rain_on && get_show_lensflare()){
        the_flare->render(glm::vec3(20,50,20));
    }

    rendering::buffer::geom->unbind();
}

void pass::geomAnimated() {
    static Shader shader { "geomAnim", "shader/anim.vs", "shader/anim.fs" };

    rendering::buffer::geom->bind();
    shader->bind();

    for (auto &[id, model] : MovableModel::movable_models) {
        if(!dynamic_cast<AnimatedModel *>(model.get())) continue;

        if(model == the_player && the_player->playerView == PlayerView::FIRST_PERSON){
            // skip active player if in 1st person view
            continue;
        }

        model->draw(shader);
    }

    if(the_player->selectedItem == globals::ModelType::sword){
        the_player->player_sword->draw(shader);
    }
    else if(the_player->selectedItem == globals::ModelType::axe){
        the_player->player_axe->draw(shader);
    }

    shader->unbind();

    rendering::buffer::geom->unbind();
}


void pass::geomScreenObj() {
    static Shader pos_norm_tc2 { "pos_norm_tc2", "shader/pos+norm+tc_screen.vs", "shader/pos+norm+tc_screen.fs" };

    rendering::buffer::screen->bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    pos_norm_tc2->bind();

    current_camera()->near = 0.0001f;
    pos_norm_tc2->uniform("proj", the_player->player_cam->proj);
    pos_norm_tc2->uniform("view", glm::mat4(1));
    //pos_norm_tc2->uniform("view", current_camera()->view);

    //the_arm->draw(pos_norm_tc2);
    //the_sword->draw(pos_norm_tc2);
    the_player->screen_item->draw(pos_norm_tc2);


    pos_norm_tc2->unbind();
    rendering::buffer::geom->unbind();

    current_camera()->near = 0.01f;
}



void pass::shadow() {
    static Shader shadow_shader{"shadow", "shader/shadow.vs", "shader/shadow.fs"};

    shadow_pass = true;

    static float factor = 1.5f;
    static float units = 1700.0f;
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(factor, units);

    buffer::shadow_fb->bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    auto shadowcam = Camera::find("shadowcam");

    shadow_shader->bind();
    shadow_shader->uniform("proj_view", shadowcam->proj * shadowcam->view);
    // TODO: draw player, movableModel
    //  the_scene->draw_shadow(shadow_shader);
    shadow_shader->unbind();

    buffer::shadow_fb->unbind();

    glDisable(GL_POLYGON_OFFSET_FILL);


    // point shadows
    GLint prev_vp[4];
    glGetIntegerv(GL_VIEWPORT, prev_vp);
    glViewport(0, 0, 256, 256);



    //draw
    for(auto p : PointLight::point_lights){
        p->draw_point_shadows();
    }


    glViewport(prev_vp[0], prev_vp[1], prev_vp[2], prev_vp[3]);

    shadow_pass = false;

}

void pass::deferred_lighting() {
    static Shader deferred_lighting_shader { "deferred_lighting", "shader/copytex.vs", "shader/deferred_lighting.fs" };

    using namespace rendering::buffer;

    lighting->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    deferred_lighting_shader->bind();

    util::setup_camera(deferred_lighting_shader, current_camera());

    // uniform point light arrays


    deferred_lighting_shader->uniform("geom.depth", 	geom->depth_texture,     0);
    deferred_lighting_shader->uniform("geom.diff",  	geom->color_textures[0], 1);
    deferred_lighting_shader->uniform("geom.pos",   	geom->color_textures[1], 2);
    deferred_lighting_shader->uniform("geom.norm",  	geom->color_textures[2], 3);
    deferred_lighting_shader->uniform("screen_depth", screen->depth_texture, 4);
    deferred_lighting_shader->uniform("shadow",     	shadow_fb->depth_texture,6);
    auto shadowcam = Camera::find("shadowcam");
    deferred_lighting_shader->uniform("shadow_proj_view", shadowcam->proj * shadowcam->view);

    deferred_lighting_shader->uniform("screen_hit_radius", screen_hit_radius);
    deferred_lighting_shader->uniform("max_hit_dist", max_hit_dist);
    deferred_lighting_shader->uniform("redness_factor", the_player->redness_factor);
    deferred_lighting_shader->uniform("player_hit", the_player->player_hit);
    deferred_lighting_shader->uniform("rayleigh_scattering_on", rayleigh_scattering_on);

    deferred_lighting_shader->uniform("sun_ambient", the_sun->ambient);
    deferred_lighting_shader->uniform("sun_diffuse", the_sun->diffuse);
    deferred_lighting_shader->uniform("sun_dir", the_sun->dir);


    deferred_lighting_shader->uniform("bias", bias);
    deferred_lighting_shader->uniform("num_p_lights", int(PointLight::point_lights.size()));

    for(int i = 0; i < (int) PointLight::point_lights.size(); i++){
        auto p = PointLight::point_lights[i];

        deferred_lighting_shader->uniform("p_light_pos[" + std::to_string(i) + "]", p->pos);
        deferred_lighting_shader->uniform("p_light_diffuse", glm::vec3(212.f / 255.f, 71.f / 255.f, 11.f / 255.f));
        deferred_lighting_shader->uniform("p_light_ambient", glm::vec3(2.f, 2.f, 2.f));

        int loc = glGetUniformLocation(deferred_lighting_shader->id, ("p_light_depthMap[" + std::to_string(i) + "]").c_str());
        glActiveTexture(GL_TEXTURE0 + 7 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, p->cube_depth);
        glUniform1i(loc, 7 + i);

        if(Lightning::the_lightning){
            if(p == Lightning::the_lightning->point_light1) {
                deferred_lighting_shader->uniform("lightning_index1", i);
            }
            else if(p == Lightning::the_lightning->point_light2) {
                deferred_lighting_shader->uniform("lightning_index2", i);
            }
            else if(p == Lightning::the_lightning->point_light3) {
                deferred_lighting_shader->uniform("lightning_index3", i);
            }
        }
    }

    deferred_lighting_shader->uniform("lightPos", PointLight::pShadow_cam->pos);
    deferred_lighting_shader->uniform("far_plane", 6.f);   //or slider

    //deferred_lighting_shader->uniform("far_plane", far_plane);
    deferred_lighting_shader->uniform("off", off);
    if(Lightning::the_lightning) {
        //deferred_lighting_shader->uniform("lightning_amb", glm::vec3(3.f));
        deferred_lighting_shader->uniform("amb_col", amb_col);
        deferred_lighting_shader->uniform("lightning_diffuse", glm::vec3(off));
        deferred_lighting_shader->uniform("far_plane_lightning", 200.f);
    }
    deferred_lighting_shader->uniform("lightning_on", Lightning::the_lightning ? true : false);

    // glDisable(GL_DEPTH_TEST);
    Quad::draw();
    // glEnable(GL_DEPTH_TEST);

    deferred_lighting_shader->unbind();

    if(rain_on) {
        particles_rain->draw();
    }
    particles_small->draw();
    particles_torch->draw();
    if(Lightning::the_lightning) {
        Lightning::the_lightning->draw();
        if(Lightning::the_lightning->lightning_finished){
            Lightning::the_lightning.reset();
        }
    }

    lighting->unbind();
}

void pass::final() {
    static Shader final_shader { "final", "shader/copytex.vs", "shader/final.fs" };

    using namespace rendering::buffer;

    final_shader->bind();

    final_shader->uniform("lighting.depth", lighting->depth_texture, 0);
    final_shader->uniform("lighting.color", lighting->color_textures[0], 1);

    glDisable(GL_DEPTH_TEST);
    Quad::draw();
    glEnable(GL_DEPTH_TEST);

    final_shader->unbind();
}

