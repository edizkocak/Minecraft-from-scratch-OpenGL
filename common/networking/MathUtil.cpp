#include <algorithm>
#include <stdexcept>
#include <random>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>
#include "../cppgl/src/globals.h"

#include "MathUtil.h"

namespace {
    std::mt19937 &getMt() {
        static std::random_device rd;
        static std::mt19937 mt(rd());
        return mt;
    }
}

float MathUtil::max(std::vector<float> v) {
    if (v.empty()) {
        throw std::runtime_error("");
    }
    return *max_element(std::begin(v), std::end(v));
}

double MathUtil::rand_0_1() {
    // source: https://stackoverflow.com/a/19666713/6334421
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(getMt());
}

double MathUtil::rand() {
    static std::uniform_real_distribution<double> dist(-1.0, 1.0);
    return dist(getMt());
}

glm::vec2 MathUtil::randVec2() {
    return {rand(), rand()};
}

glm::vec3 MathUtil::randVec3() {
    return {rand(), rand(), rand()};
}

float MathUtil::yawFromDir(glm::vec3 dir) {
    return glm::orientedAngle(VEC_FORWARD, dir, VEC_UP);

//    glm::vec3 yawedSide = glm::normalize(glm::rotate(
//            VEC_RIGHT, yaw, VEC_UP
//    ));
//    glm::vec3 yawed = glm::normalize(glm::rotate(
//            VEC_FORWARD, yaw, VEC_UP
//    ));
//
//    const float pitch = glm::orientedAngle(yawed, dir, yawedSide);

//    const glm::vec3 flippedDir = {dir.y, dir.z, dir.x};
//    float pitch = glm::orientedAngle(VEC_FORWARD, flippedDir, VEC_UP) + float(M_PI_2);
//
//    if(pitch > M_PI_4 && pitch <= M_PI_2){
//        pitch = -pitch + float(M_PI_2);
//    }

//    const float pitch = glm::orientedAngle(VEC_FORWARD, dir, VEC_RIGHT);
}
