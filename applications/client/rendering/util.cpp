#include "util.h"

#include <cppgl.h>

using namespace glm;
using namespace rendering;

const DirectionalLight rendering::default_light {
    normalize(vec3 { -0.4f, -0.6f, -0.4f }), /* dir      */
    vec3 { 0.7f, 0.7f, 0.7f },               /* ambient  */
    vec3 { 1.0f, 1.0f, 1.0f },               /* diffuse  */
    vec3 { 1.0f, 1.0f, 1.0f },               /* specular */
};

const Point_Light rendering::point_light {
    vec3 { 3.0f, 34.0f, 16.0f },             /* pos      */
    vec3 { 0.1f, 0.1f, 0.1f },               /* ambient  */
    vec3 { 0.9f, 0.7f, 0.5f },               /* diffuse  */
    vec3 { 1.0f, 1.0f, 1.0f },               /* specular */
    50,                                      /* radius */
};

void util::setup_camera(const Shader& shader, const Camera& cam) {
    shader->uniform("cam.near", cam->near);
    shader->uniform("cam.far",  cam->far);
    shader->uniform("cam.pos",  cam->pos);
    shader->uniform("cam.dir",  cam->dir);
}

void util::setup_light(const Shader& shader, const DirectionalLight& light) {
    shader->uniform("light.dir",      light.dir);
    shader->uniform("light.ambient",  light.ambient);
    shader->uniform("light.diffuse",  light.diffuse);
    shader->uniform("light.specular", light.specular);
    shader->uniform("directional_light", true);
}

void util::setup_light(const Shader& shader, const Point_Light& light) {
    shader->uniform("p_light.pos",      light.pos);
    shader->uniform("p_light.ambient",  light.ambient);
    shader->uniform("p_light.diffuse",  light.diffuse);
    shader->uniform("p_light.specular", light.specular);
    shader->uniform("p_light.radius", light.radius);
    shader->uniform("directional_light", false);
}
