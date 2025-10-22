#ifndef GRAPA_SOUNDMANAGER_H
#define GRAPA_SOUNDMANAGER_H

#include <vector>
#include <filesystem>
#include "irrKlang-64bit-1.6.0/include/irrKlang.h"

class SoundManager {
public:
    SoundManager();
    ~SoundManager();

protected:
    irrklang::ISound *music;
    std::filesystem::path musicRelaxedPath;

    irrklang::ISoundEngine *engine;
};

#endif //GRAPA_SOUNDMANAGER_H
