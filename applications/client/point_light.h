#pragma once
#include <cppgl.h>


class PointLight {
public:
    PointLight(glm::vec3 pos, bool lightning);
    ~PointLight(){}

    void camera_face(int faceIndex);
    void draw_point_shadows();

    static std::vector<std::shared_ptr<PointLight>> point_lights;
    static Camera pShadow_cam;
    static glm::mat4 shadowMatrices[6];

    GLuint pShadow_buffer;
    GLuint cube_depth;
    int frame_count = 0;

    glm::vec3 pos;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    int radius;
    bool lightning;

    inline float random_float() { return (float(rand() % 32768) / 16384.0f) - 1.0f; }
};



