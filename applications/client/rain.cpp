#include "rain.h"
#include "rendering/buffer.h"
#include "model/player.h"
#include <cmath>

extern std::shared_ptr<Player> the_player;
extern float rain_size;


Rain::Rain(unsigned int N,
        float particle_size)
        : Base(N, particle_size), offset(N)
{
}

void Rain::add(const glm::vec3& pos, const glm::vec3& dir, const glm::vec4& col, int alive_for_ms) {
    const uint32_t at = end;
    if (advance(end) == start)
        advance(start);
    position[at] = pos;
    direction[at] = dir;
    lifetime[at] = alive_for_ms;
    glm::vec3 diff = pos - the_player->getPos();
    offset[at] = glm::vec2(diff.x, diff.z);
    color = col;
}

void Rain::update(){
    static const float MAX_HEIGHT = 50.f;

    const auto dt = (float) Context::frame_time();
    const uint32_t bound = (end >= start) ? end : end+N;
    for (uint32_t i = start; i < bound; ++i) {
        const uint32_t idx = i % N;
        lifetime[idx] = 100;
        //if (lifetime[idx] <= 0 && idx == start)
        //    advance(start);
        float speed_factor = 0.0162f;
        position[idx] = position[idx] + direction[idx] * dt * speed_factor;
        if(position[idx].y <= 0){
            position[idx].y = MAX_HEIGHT - fmod(position[idx].y,MAX_HEIGHT);
            position[idx][0] = the_player->getPos()[0] + offset[idx][0];
            position[idx][2] = the_player->getPos()[2] + offset[idx][1];
        }
        else {
            float height_diff = abs(the_player->getPos().y - position[idx][1]);
            if(height_diff > 10) {
                position[idx][0] = the_player->getPos()[0] + offset[idx][0];
                position[idx][2] = the_player->getPos()[2] + offset[idx][1];
            }
        }
    }
    mesh->update_vertex_buffer(vbo_id_pos, position.data());
    mesh->update_vertex_buffer(vbo_id_life, lifetime.data());
}


void Rain::draw() {
    static Texture2D rain_mask = Texture2D("rain_mask", globals::renderData() / "lightning_bolt" / "rain.png");

    // draw pass as points
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);

    draw_shader->bind();
    auto cam = current_camera();
    draw_shader->uniform("view", cam->view);
    draw_shader->uniform("proj", cam->proj);
    draw_shader->uniform("near_far", glm::vec2(cam->near, cam->far));
    draw_shader->uniform("screenres", glm::vec2(Context::resolution()));
    draw_shader->uniform("particle_size", rain_size);
    draw_shader->uniform("gbuf_depth", rendering::buffer::geom->depth_texture, 0);
    draw_shader->uniform("color", color);
    draw_shader->uniform("rain_mask", rain_mask, 1);
    draw_shader->uniform("rain", true);

    mesh->bind(draw_shader);
    if (start < end) {
        glDrawElementsBaseVertex(GL_POINTS, end - start, GL_UNSIGNED_INT, 0, start);
    }
    else if (end < start) {
        glDrawElementsBaseVertex(GL_POINTS, N - end - 1, GL_UNSIGNED_INT, 0, start);
        glDrawElementsBaseVertex(GL_POINTS, end, GL_UNSIGNED_INT, 0, 0);
    }
    mesh->unbind();
    draw_shader->unbind();

    glDisable(GL_POINT_SPRITE);
    glDisable(GL_PROGRAM_POINT_SIZE);

    // splatting pass using a geometry shader
    splat_shader->bind();
    splat_shader->uniform("view", cam->view);
    splat_shader->uniform("proj", cam->proj);
    splat_shader->uniform("near_far", glm::vec2(cam->near, cam->far));
    splat_shader->uniform("screenres", glm::vec2(Context::resolution()));
    splat_shader->uniform("radius", rain_size * 5.f);
    splat_shader->uniform("gbuf_depth", rendering::buffer::geom->depth_texture, 0);
    splat_shader->uniform("gbuf_diff", rendering::buffer::geom->color_textures[0], 1);
    splat_shader->uniform("gbuf_pos", rendering::buffer::geom->color_textures[1], 2);
    splat_shader->uniform("gbuf_norm", rendering::buffer::geom->color_textures[2], 3);
    splat_shader->uniform("rain", true);

    mesh->bind(splat_shader);
    if (start < end) {
        glDrawElementsBaseVertex(GL_POINTS, end - start, GL_UNSIGNED_INT, 0, start);
    }
    else if (end < start) {
        glDrawElementsBaseVertex(GL_POINTS, N - end - 1, GL_UNSIGNED_INT, 0, start);
        glDrawElementsBaseVertex(GL_POINTS, end, GL_UNSIGNED_INT, 0, 0);
    }
    mesh->unbind();

    splat_shader->unbind();


    //reset for other render calls
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
