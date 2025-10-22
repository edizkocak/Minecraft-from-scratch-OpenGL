#ifndef GRAPA_ASTAR_H
#define GRAPA_ASTAR_H

#include <cppgl.h>
#include <set>
#include <algorithm>
#include <deque>
#include "SvMap.h"

namespace AStar{
    void init(SvMap* map);

    std::vector<glm::ivec3> neighborsOf(const glm::ivec3 &pos, int heightInBlocks);

    /**
     * If a path exists, then return [start, ..., goal].
     * <br>
     * Otherwise: [start, ..., closestToGoal].
     */
    std::deque<glm::ivec3> search(glm::ivec3 start, glm::ivec3 goal, float height);
}

#endif //GRAPA_ASTAR_H
