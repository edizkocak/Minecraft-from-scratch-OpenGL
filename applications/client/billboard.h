#pragma once

#include <cppgl.h>

void init_textures();

class Billboard
{
public:
    Billboard();
    ~Billboard();

    void draw();
    glm::vec3 get_sun_pos();
    void update();
    void load_textures();

    glm::mat4 model;
	static std::vector<Drawelement> prototype;
    std::vector<Texture2D> tex;
    Timer timer;
    float cnt;

};

class Grass {
    Grass();
    ~Grass();

    void draw();
    static std::vector<Drawelement> prototype;
    glm::mat4 model;


};

