#ifndef GRAPA_GLOBALS_H
#define GRAPA_GLOBALS_H

#include <filesystem>

namespace globals {
    extern std::filesystem::path renderData();
    static constexpr bool DEBUG = true;
    extern float HIDPI_SCALE;

    static constexpr int NUM_ENEMIES = 50;
    static constexpr int NUM_ANIMALS = 50;

    static constexpr int SERVER_INTERVAL = 10; // ms
    static constexpr float FARPLANE = 50.f; // clouds are at height 40
    static constexpr float CULLING_FAR = 20.f; // clouds are at height 40

    static constexpr float hp {100};
    static constexpr int chunkDim {8};
    static constexpr float viewDistance = chunkDim * 4.5;
    static constexpr int maxMovableObjId = 10000;

    static constexpr float CLOUD_CHANGE_TIME = 15 * 1000.f;
    static constexpr float mouseYFactor = 1.f; // to make the camera movement in y-direction a bit slower

    enum class AnimationType {
        // TODO
        WALKING,
        STANDING,
        DIGGING,
        HITTING,
        WALKING_AND_DIGGING,
        WALKING_AND_HITTING,
    };

    enum class ModelType {
        air,

        water,
        wood,
        dirt,
        grass,
        leaf,
        stone,
        torch,


        cloud0,
        cloud1,
        cloud2,
        cloud3,

        player,
        dog,
        horse,
        zombie,
        skeleton,

        arm,
        sword_screen,
        axe_screen,

        sword,
        axe
    };
}

#if defined(IVEC3_INVALID_POS) || defined(VEC3_INVALID_POS) || defined(VEC2_INVALID_POS)
#error "expected *VEC*_INVALID_POS not to exist"
#endif
#define IVEC3_INVALID_POS glm::ivec3(-1,-1,-1)
#define VEC3_INVALID_POS glm::vec3(-1,-1,-1)
#define VEC2_INVALID_POS glm::vec2(-1,-1)

#if defined(IVEC_FORWARD) || defined(VEC_FORWARD) || defined(IVEC_BACKWARD) || defined(VEC_BACKWARD)
#error "expected IVEC_* not to exist"
#endif
#define IVEC_FORWARD glm::ivec3(1,0,0)
#define VEC_FORWARD glm::vec3(1,0,0)
#define IVEC_BACKWARD glm::ivec3(-1,0,0)
#define VEC_BACKWARD glm::vec3(-1,0,0)

#if defined(IVEC_RIGHT) || defined(VEC_RIGHT) || defined(IVEC_LEFT) || defined(VEC_LEFT)
#error "expected IVEC_* not to exist"
#endif
#define IVEC_RIGHT glm::ivec3(0,0,1)
#define VEC_RIGHT glm::vec3(0,0,1)
#define IVEC_LEFT glm::ivec3(0,0,-1)
#define VEC_LEFT glm::vec3(0,0,-1)

#if defined(IVEC_UP) || defined(VEC_UP) || defined(IVEC_DOWN) || defined(VEC_DOWN)
#error "expected IVEC_* not to exist"
#endif
#define IVEC_UP glm::ivec3(0,1,0)
#define VEC_UP glm::vec3(0,1,0)
#define IVEC_DOWN glm::ivec3(0,-1,0)
#define VEC_DOWN glm::vec3(0,-1,0)

#endif //GRAPA_GLOBALS_H
