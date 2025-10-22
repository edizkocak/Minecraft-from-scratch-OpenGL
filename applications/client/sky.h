#pragma once

#include <string>
#include<cppgl.h>


class Sky {
public:

    GLuint day;
    GLuint night;

    explicit Sky(float scale);
    ~Sky()= default;

    void setup_skybox();
    void draw();
    void update();

    glm::mat4 model;
    static std::vector<Drawelement> prototype;
};

class Sun {
public:
   

    GLuint day;
    GLuint night;



    Sun();
    ~Sun()= default;

  
    void draw();
    void update();
    glm::vec3 get_sun_pos();

    glm::mat4 model;
     glm::mat4 rot;
     glm::vec2 midpoint;

     glm::vec3 ambient = glm::vec3(0.5f);
     glm::vec3 diffuse = glm::vec3(1.f);
     glm::vec3 dir = glm::vec3(0,1,0);


    static std::vector<Drawelement> sun_prototype;
};
