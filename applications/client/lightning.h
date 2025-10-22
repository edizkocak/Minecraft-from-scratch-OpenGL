#pragma once
#include <cppgl.h>
#include "point_light.h"




class Lightning
{
public:
    explicit Lightning(glm::vec2 pos);
    ~Lightning();


    glm::vec3 get_sun_pos();
    static void init_lightning();
    // triggered by server, pass a clouds ivec xz-pos as argument
    // the lightning will spawn inside this cloud (at set height 40)
    static void spawn_lightning(glm::ivec2 pos);
    static void spawn_lightning(int cloudId);


    void update();
    void draw();

    // the lightning object
    static std::shared_ptr<Lightning> the_lightning;


    std::shared_ptr<PointLight> point_light1;
    std::shared_ptr<PointLight> point_light2;
    std::shared_ptr<PointLight> point_light3;

    glm::vec2 curr_dir;

    glm::mat4 model;
    glm::mat4 rot;
    static std::vector<Drawelement> prototype;
    static std::vector<Texture2D> tex;
    Timer timer;
    int cnt = 0;

    bool lightning_finished;

};

