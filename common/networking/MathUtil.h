#ifndef GRAPA_MATHUTIL_H
#define GRAPA_MATHUTIL_H

#include <vector>
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace MathUtil {
    float max(std::vector<float> v);

    /**
     * @return in the range [0, 1)
     */
    double rand_0_1();

    /**
     * @return in range [-1, 1)
     */
    double rand();
    /**
     * @return in range [-1, 1) x [-1, 1)
     */
    glm::vec2 randVec2();
    /**
     * @return in range [-1, 1) x [-1, 1) x [-1, 1)
     */
    glm::vec3 randVec3();

    /**
     * @param dir normalized direction
     */
    float yawFromDir(glm::vec3 dir);
}

#endif //GRAPA_MATHUTIL_H
