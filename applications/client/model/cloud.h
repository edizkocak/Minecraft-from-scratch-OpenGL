#include "model.h"
#include "../map/block.h"
#include "cppgl/src/cppgl.h"

using Base = Model;

class Cloud : public Model {
public:
    Cloud(globals::ModelType type, glm::ivec2 pos);

    inline static std::map<int, std::shared_ptr<Cloud>> all_clouds;

    inline static Timer darkness_timer;
    inline static bool cloud_darkening = false;
    inline static bool cloud_whitening = false;
    inline static float cloud_darkness_factor = 0;

    static constexpr glm::vec3 dir{1,0,1};

    static void darken_clouds();
    static void whiten_clouds();
    static void rain_around_player(glm::vec3 pos);

    static void updateDarkness();
    void update() override;
    void updateVisibility() override;
    void draw(Shader& shader) override;
};
