#include "particles.h"


class Rain : public Particles {
public:
    using Base = Particles;

    Rain(unsigned int N, float particle_size);

    std::vector<glm::vec2> offset;

    void add(const glm::vec3& pos, const glm::vec3& dir, const glm::vec4& col, int alive_for_ms) override;
    void update() override;
    void draw() override;
};



