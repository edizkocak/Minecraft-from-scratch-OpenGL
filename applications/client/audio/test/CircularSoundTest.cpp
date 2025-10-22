#include "CircularSoundTest.h"

#include "../SoundUtil.h"
#include "query.h"
#include "irrKlang-64bit-1.6.0/include/irrKlang.h"
#include <iostream>
#include <thread>
#include <chrono>

#include "cppgl/src/cppgl.h"

using namespace std;
namespace fs = std::filesystem;
using namespace irrklang;

[[noreturn]] void CircularSoundTest::loop() {
    int options = ESEO_DEFAULT_OPTIONS;
    if ((options & ESEO_USE_3D_BUFFERS) != 0) {
        // ESEO_USE_3D_BUFFERS is enabled

        // disable ESEO_USE_3D_BUFFERS if hardware 3d audio is slow on the system
        cout << "disabling ESEO_USE_3D_BUFFERS" << endl;
        options ^= ESEO_USE_3D_BUFFERS;
        if ((options & ESEO_USE_3D_BUFFERS) != 0) {
            throw runtime_error("expected ESEO_USE_3D_BUFFERS to be disabled");
        }
    }

    // start the sound engine
    ISoundEngine *engine = createIrrKlangDevice(ESOD_AUTO_DETECT, options);
    if (!engine) {
        throw runtime_error("Could not start sound engine!");
    }

    // update camera/listener position
    {
        irrklang::vec3df position(0, 0, 0);        // position of the listener
        irrklang::vec3df lookDirection(1, 0, 0);   // direction the listener looks into
        irrklang::vec3df velPerSecond(0, 0, 0);    // velocity per second describes the speed of the listener and
                                                            // is only needed for doppler effects.
        irrklang::vec3df upVector(0, 1, 0);        // where 'up' is in your 3D scene

        engine->setListenerPosition(position, lookDirection, velPerSecond, upVector);
    }

    Timer t;
    const irrklang::vec3df position = SoundUtil::toVec3(posAt(0));

    // start sound paused
    irrklang::ISound *snd = engine->play3D(soundFile().string().c_str(), position, true, true);
    if (!snd) {
        throw runtime_error("Could not play sound!");
    }

    // default minimum distance is 1
    // a loud sound
    snd->setMinDistance(10.0f);
    // unpause the sound
    snd->setIsPaused(false);
    t.begin();

    while (true) {
        const glm::vec3 pos = posAt(t.look());
        const irrklang::vec3df iPos = SoundUtil::toVec3(pos);
        snd->setPosition(iPos);

//        cout << pos << endl;
        cout << iPos.X << ", " << iPos.Y << ", " << iPos.Z << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    snd->drop(); // delete sound
    engine->drop(); // delete engine
}

glm::vec3 CircularSoundTest::posAt(double ms) {
    const double x = ms / 5000;
    const double distance = 50;
    return {
            distance * cos(x),
            0,
            distance * sin(x)
    };
}

std::filesystem::path CircularSoundTest::soundFile() {
    // Keep your application simple
    // play2D("filename.mp3")
    // irrKlang will never load a file twice

    // If you can choose which audio file format is the primary one for your application,
    // use .OGG files, instead of for example .MP3 files. irrKlang uses a lot less memory
    // and CPU power when playing .OGGs.
    return globals::renderData() / "sounds" / "rain" / "sound.ogg";
}
