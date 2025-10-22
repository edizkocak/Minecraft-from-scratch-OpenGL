#ifndef GRAPA_SOUNDTEST_H
#define GRAPA_SOUNDTEST_H

#include "irrKlang-64bit-1.6.0/include/irrKlang.h"
#include <filesystem>

class SoundTest{
public:
    static void fadeOut();

    /**
     * start sound engine
     * play a sound
     * stop engine when playback has finished
     */
    static void startPlayStop();
    /**
     * start sound engine
     * play a sound
     * @return sound engine
     */
    static irrklang::ISoundEngine *startPlay();

    static std::filesystem::path soundFile();
    static std::filesystem::path songFile();
};

#endif //GRAPA_SOUNDTEST_H
