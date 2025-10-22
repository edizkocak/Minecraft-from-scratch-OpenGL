#ifndef GRAPA_MESSAGE_H
#define GRAPA_MESSAGE_H

#include "../nlohmann_json/include/nlohmann/json.hpp"
#include <cppgl.h>

namespace glm {
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(glm::vec2, x, y)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(glm::vec3, x, y, z)

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(glm::ivec2, x, y)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(glm::ivec3, x, y, z)
}

namespace msg {
    enum class MsgType {
        SV_LOGIN,
        SV_LOGOUT,
        SV_UPDATE_PLAYER,
        SV_UPDATE_HP,
        SV_DIG,
        SV_PLACE,
        SV_ADD_PROJECTILE,
        SV_UPDATE_PROJECTILE,

        CL_ADD_PLAYER,
        CL_FREEZE,
        CL_UPDATE_INVENTORY,
        CL_ADD_MOVABLE,
        CL_UPDATE_ANIMATED,
        CL_UPDATE_HP,
        CL_UPDATE_PROJECTILE,
        CL_UPDATE_BLOCK,
        CL_UPDATE_CLOUD,
        CL_THUNDERSTORM,
        CL_LIGHTING,
        CL_EXIT,

        MESSAGE_TYPE_ERROR,
    };

    class Msg {
    public:
        Msg() = default;
        explicit Msg(MsgType type) : type(type) {
        }

        MsgType type = MsgType::MESSAGE_TYPE_ERROR;
        nlohmann::json json;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Msg, type, json)
}

#define GRAPRA_TO_JSON_1(v) \
    msg.json[#v] = v;
#define GRAPRA_TO_JSON_2(v1, v2) \
    GRAPRA_TO_JSON_1(v1) \
    GRAPRA_TO_JSON_1(v2)
#define GRAPRA_TO_JSON_3(v1, v2, v3) \
    GRAPRA_TO_JSON_2(v1,v2) \
    GRAPRA_TO_JSON_1(v3)
#define GRAPRA_TO_JSON_4(v1, v2, v3, v4) \
    GRAPRA_TO_JSON_3(v1,v2,v3) \
    GRAPRA_TO_JSON_1(v4)
#define GRAPRA_TO_JSON_5(v1, v2, v3, v4, v5) \
    GRAPRA_TO_JSON_4(v1,v2,v3,v4) \
    GRAPRA_TO_JSON_1(v5)
#define GRAPRA_TO_JSON_6(v1, v2, v3, v4, v5, v6) \
    GRAPRA_TO_JSON_5(v1,v2,v3,v4,v5) \
    GRAPRA_TO_JSON_1(v6)
#define GRAPRA_TO_JSON_7(v1, v2, v3, v4, v5, v6, v7) \
    GRAPRA_TO_JSON_6(v1,v2,v3,v4,v5,v6) \
    GRAPRA_TO_JSON_1(v7)

#define GRAPRA_FROM_JSON(v, type) \
    const auto v = msg.json.at(#v).get<type>();

// TODO uncomment to debug
// map ModelType values to JSON as strings
//namespace globals {
//    NLOHMANN_JSON_SERIALIZE_ENUM(AnimationType, {
//        { AnimationType::WALKING, "WALKING" },
//        { AnimationType::STANDING, "STANDING" },
//        { AnimationType::DIGGING, "DIGGING" },
//    })
//
//    NLOHMANN_JSON_SERIALIZE_ENUM(ModelType, {
//        { ModelType::air, "air" },
//
//        { ModelType::water, "water" },
//        { ModelType::wood, "wood" },
//        { ModelType::dirt, "dirt" },
//        { ModelType::grass, "grass" },
//        { ModelType::leaf, "leaf" },
//        { ModelType::stone, "stone" },
//        { ModelType::torch, "torch" },
//
//        { ModelType::cloud0, "cloud0" },
//        { ModelType::cloud1, "cloud1" },
//        { ModelType::cloud2, "cloud2" },
//        { ModelType::cloud3, "cloud3" },
//
//        { ModelType::player, "player" },
//        { ModelType::dog, "dog" },
//        { ModelType::horse, "horse" },
//        { ModelType::zombie, "zombie" },
//        { ModelType::skeleton, "skeleton" },
//    })
//}

#endif //GRAPA_MESSAGE_H
