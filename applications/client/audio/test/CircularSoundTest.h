#ifndef GRAPA_CIRCULARSOUNDTEST_H
#define GRAPA_CIRCULARSOUNDTEST_H

#include <filesystem>
#include "glm/vec3.hpp"

class CircularSoundTest {
public:
    [[noreturn]] static void loop();

protected:
    static glm::vec3 posAt(double ms);

    static std::filesystem::path soundFile();
};


#endif //GRAPA_CIRCULARSOUNDTEST_H
