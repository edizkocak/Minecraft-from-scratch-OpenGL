#ifndef GRAPA_SVMOVABLE_H
#define GRAPA_SVMOVABLE_H

#include "globals.h"
#include "SvMap.h"
#include <cppgl.h>
#include <utility>
#include <deque>

class SvMovable {
public:
    SvMovable(int id, globals::ModelType type)
            : type(type), id(id) {
    }

//    [[nodiscard]] bool isEnemy() const {
//        return type == globals::ModelType::skeleton || type == globals::ModelType::zombie;
//    }

    static void init(SvMap* map);

    globals::ModelType type;
    int id;
};

class SvNpc : public SvMovable {
public:
    /**
     * @param pos Block Position. SvNpc will be added to the middle of that block.
     */
    SvNpc(int id, globals::ModelType type, glm::vec3 pos, int ms)
            : SvMovable(id, type), pos(pos + glm::vec3(0.5f, 0, 0.5f)), animationStartMs(ms) {
    }

    void update(double ms, int i);
protected:
    void move(double ms);
public:
    void updatePath(double ms);

    double lastAttackMs{0};

    float hp{globals::hp};
    float yaw{0};

    glm::vec3 velocity{0};
    glm::vec3 pos;

    double pathStartMs{std::numeric_limits<int>::min()};
    std::deque<glm::ivec3> path{};

    int animationStartMs;
    globals::AnimationType animationType = globals::AnimationType::WALKING;

    inline static std::map<int, std::shared_ptr<SvNpc>> npcs;
};

class SvPlayer : public SvMovable {
public:
    /**
     * @param pos Block Position. SvNpc will be added to the middle of that block.
     */
    SvPlayer(int id, globals::ModelType type, glm::vec3 pos, std::string name, int i)
            : SvMovable(id, type), pos(pos + glm::vec3(0.5f, 0, 0.5f)), i(i), name(std::move(name)) {
    }

    void update();

    static std::shared_ptr<SvPlayer> closestPlayer(glm::vec3 pos);

    bool frozen = false;
    float hp = globals::hp;
    glm::vec3 pos;

    int i;
    std::string name;

    inline static int numPlayers;
    inline static std::vector<std::shared_ptr<SvPlayer>> playerList;
    inline static std::map<int, std::shared_ptr<SvPlayer>> players;
};

#endif //GRAPA_SVMOVABLE_H
