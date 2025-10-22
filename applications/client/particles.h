#pragma once

#include <cppgl.h>

class Particles {

    std::vector<uint32_t> index_buffer;

protected:
    static Shader draw_shader;
    Mesh mesh;
    uint32_t start;
    uint32_t end;
    const float particle_size;
    glm::vec3 color;
    static Shader splat_shader;
    std::vector<glm::vec3> position;
    std::vector<int> lifetime;
    bool torch;
 
    uint32_t advance(uint32_t& pointer);

    std::vector<glm::vec3> direction;
    uint32_t vbo_id_pos;
    uint32_t vbo_id_life;
    const uint32_t N;
public:
    Particles(unsigned int N, float particle_size);
    ~Particles();

    virtual void add(const glm::vec3& pos, const glm::vec3& dir, const glm::vec4& col, int alive_for_ms);
    virtual void update();
    virtual void draw();
    void set_torch();
    void add_torch_particles(glm::vec3 torch_pos);
};
