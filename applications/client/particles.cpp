#include "particles.h"
#include "rendering/buffer.h"
#include "rendering/util.h"

#include <cppgl.h>
#include <iostream>

Shader Particles::draw_shader;
Shader Particles::splat_shader;

Particles::Particles(unsigned int N, float particle_size) : index_buffer(N), start(0), end(0), particle_size(particle_size), position(N), lifetime(N, 0), direction(N), N(N) {
    static unsigned idx = 0;
    mesh = Mesh(std::string("particle-data") + std::to_string(idx++));
    vbo_id_pos = mesh->add_vertex_buffer(GL_FLOAT, 3, N, position.data(), GL_DYNAMIC_DRAW);
    vbo_id_life = mesh->add_vertex_buffer(GL_INT, 1, N, lifetime.data(), GL_DYNAMIC_DRAW);
    for (unsigned i = 0; i < N; ++i) index_buffer[i] = i;
    mesh->add_index_buffer(N, index_buffer.data());

    torch = false;

    // lazy init static stuff
    if (!draw_shader)
        draw_shader = Shader("particle-draw", "shader/particle-flare.vs", "shader/particle-flare.fs");
    if (!splat_shader)
        splat_shader = Shader("particle-splat", "shader/particle-splat.vs", "shader/particle-splat.gs", "shader/particle-splat.fs");
}

Particles::~Particles() = default;

uint32_t Particles::advance(uint32_t& pointer) {
    pointer = (pointer + 1) % N;
    return pointer;
}

void Particles::add(const glm::vec3& pos, const glm::vec3& dir, const glm::vec4& col, int alive_for_ms) {
    const uint32_t at = end;
    if (advance(end) == start)
        advance(start);
    position[at] = pos;
    direction[at] = dir;
    lifetime[at] = alive_for_ms;
    color = col;
}

void Particles::update() {
    const float dt = (float) Context::frame_time();
    const int bound = (end >= start) ? end : end+N;
    for (int i = (int) start; i < bound; ++i) {
        const uint32_t idx = i % N;
        lifetime[idx] -= int(dt);
        if (lifetime[idx] <= 0 && idx == start)
            advance(start);
        position[idx] = position[idx] + direction[idx] * dt * 0.0066f;
    }
    mesh->update_vertex_buffer(vbo_id_pos, position.data());
    mesh->update_vertex_buffer(vbo_id_life, lifetime.data());
}

void Particles::draw() {
    // draw pass as points
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    static Texture2D torch_mask0 = Texture2D("torch_mask0", globals::renderData() / "lightning_bolt" / "torch0.png");
    static Texture2D torch_mask1 = Texture2D("torch_mask1", globals::renderData() / "lightning_bolt" / "torch1.png");
    static Texture2D torch_mask2 = Texture2D("torch_mask2", globals::renderData() / "lightning_bolt" / "torch3.png");
    static Texture2D torch_mask3 = Texture2D("torch_mask3", globals::renderData() / "lightning_bolt" / "torch4.png");
    Texture2D torch_mask = torch_mask3;
    float r = ((float) rand() / RAND_MAX);
    if(r > 0.5f){torch_mask = torch_mask0;}

    glDepthMask(GL_FALSE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);

    draw_shader->bind();
    auto cam = current_camera();
    draw_shader->uniform("view", cam->view);
    draw_shader->uniform("proj", cam->proj);
    draw_shader->uniform("near_far", glm::vec2(cam->near, cam->far));
    draw_shader->uniform("screenres", glm::vec2(Context::resolution()));
    draw_shader->uniform("particle_size", particle_size);
    draw_shader->uniform("gbuf_depth", rendering::buffer::geom->depth_texture, 0);
    draw_shader->uniform("color", color);
    draw_shader->uniform("rain", false);
    draw_shader->uniform("rain", false);
  // std::cout << torch << std::endl;
    draw_shader->uniform("torch", torch);

    


    mesh->bind(draw_shader);
    if (start < end) {
        glDrawElementsBaseVertex(GL_POINTS, end - start, GL_UNSIGNED_INT, 0, start);
        draw_shader->uniform("torch_mask", torch_mask, 2);
         r = ((float) rand() / RAND_MAX);
         draw_shader->uniform("random", r);
    }
    else if (end < start) {
        glDrawElementsBaseVertex(GL_POINTS, N - end - 1, GL_UNSIGNED_INT, 0, start);
        glDrawElementsBaseVertex(GL_POINTS, end, GL_UNSIGNED_INT, 0, 0);
        draw_shader->uniform("torch_mask", torch_mask, 2);
         r = ((float) rand() / RAND_MAX);
         //(random * 2 - 1 ) * 0.2
         draw_shader->uniform("random", r);
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
    splat_shader->uniform("radius", particle_size * 5.f);
    splat_shader->uniform("gbuf_depth", rendering::buffer::geom->depth_texture, 0);
    splat_shader->uniform("gbuf_diff", rendering::buffer::geom->color_textures[0], 1);
    splat_shader->uniform("gbuf_pos", rendering::buffer::geom->color_textures[1], 2);
    splat_shader->uniform("gbuf_norm", rendering::buffer::geom->color_textures[2], 3);
    splat_shader->uniform("rain", false);
    splat_shader->uniform("torch", torch);

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

void Particles::set_torch(){

    torch = true;

    }

void Particles::add_torch_particles(glm::vec3 torch_pos){
  
     float FLAME_RADIUS = 0.2f;

    for(int i = 0; i < 8; i++) {
        float r = FLAME_RADIUS * sqrt(((float) rand() / RAND_MAX));
        float theta = ((float) rand() / RAND_MAX) * 2.f * M_PI;
        add(
                glm::vec3(
                        torch_pos.x + 0.5f + r * cos(theta),
                        torch_pos.y + 0.6f  + ((float) rand() / RAND_MAX) * 0.2,
                        torch_pos.z + 0.5f  + r * sin(theta)

                ),
                glm::vec3(0, 0.2, 0),
                glm::vec4(1.f,0.f,0.f,1.f),
                200
        );
    }

    FLAME_RADIUS = 0.14f;

    for(int i = 0; i < 3; i++) {
        float r = FLAME_RADIUS * sqrt(((float) rand() / RAND_MAX));
        float theta = ((float) rand() / RAND_MAX) * 2.f * M_PI;
        add(
                glm::vec3(
                        torch_pos.x + 0.5f + r * cos(theta),
                        torch_pos.y + 0.9f  + ((float) rand() / RAND_MAX) * 0.1,
                        torch_pos.z + 0.5f  + r * sin(theta)

                ),
                glm::vec3(0, 0.3, 0),
                glm::vec4(1.f,0.f,0.f,1.f),
                100
        );
    }

     FLAME_RADIUS = 0.07f;

    for(int i = 0; i < 1; i++) {
        float r = FLAME_RADIUS * sqrt(((float) rand() / RAND_MAX));
        float theta = ((float) rand() / RAND_MAX) * 2.f * M_PI;
        add(
                glm::vec3(
                        torch_pos.x + 0.5f + r * cos(theta),
                        torch_pos.y + 1.f  + ((float) rand() / RAND_MAX) * 0.1,
                        torch_pos.z + 0.5f  + r * sin(theta)

                ),
                glm::vec3(0, 0.1, 0),
                glm::vec4(0.5f,0.f,0.f,1.f),
                100
        );
    }
     FLAME_RADIUS = 0.14f;

    for(int i = 0; i < 3; i++) {
        float r = FLAME_RADIUS * sqrt(((float) rand() / RAND_MAX));
        float theta = ((float) rand() / RAND_MAX) * 2.f * M_PI;
        add(
                glm::vec3(
                        torch_pos.x + 0.5f + r * cos(theta),
                        torch_pos.y + 0.55f  + ((float) rand() / RAND_MAX) * 0.1,
                        torch_pos.z + 0.5f  + r * sin(theta)

                ),
                glm::vec3(0, 0.3, 0),
                glm::vec4(1.f,0.f,0.f,1.f),
                100
        );
    }
}

