#pragma once

#include <cppgl.h>

namespace rendering {
    struct Point_Light {
        glm::vec3 pos;

        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float radius;
    };

    struct DirectionalLight {
        glm::vec3 dir;

        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
    };

    extern const DirectionalLight default_light;
    extern const Point_Light point_light;
}

namespace rendering::util {
    void setup_camera(const Shader& shader, const Camera& cam);
    void setup_light(const Shader&, const DirectionalLight&);
    void setup_light(const Shader& shader, const Point_Light& light);
}
