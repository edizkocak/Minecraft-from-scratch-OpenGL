#include "SoundTest.h"
#include "cppgl/src/cppgl.h"
#include "irrKlang-64bit-1.6.0/include/irrKlang.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
namespace fs = std::filesystem;
using namespace irrklang;

void SoundTest::fadeOut(){
    // start the sound engine with default parameters
    ISoundEngine *engine = createIrrKlangDevice();
    if (!engine) {
        throw runtime_error("Could not start sound engine!");
    }

    // play some sound stream, looped
    irrklang::ISound *snd = engine->play2D(songFile().string().c_str(), true, false, true, ESM_NO_STREAMING);

    // change volume from MAX_VOL=1 to MIN_VOL=0
    int steps = 100;
    for(int i = steps; i>=0; i--){
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        snd->setVolume(ik_f32(i)/ik_f32(steps));
    }

    snd->drop();
    engine->drop(); // delete engine
}

void SoundTest::startPlayStop() {
    // start the sound engine with default parameters
    ISoundEngine *engine = createIrrKlangDevice();
    if (!engine) {
        throw runtime_error("Could not start sound engine!");
    }

    // play some sound stream, looped
    // returns pointer if
    //   'track', 'startPaused' or 'enableSoundEffects' have been	 set to true
    irrklang::ISound *snd = engine->play2D(soundFile().string().c_str(), false, false, true, ESM_NO_STREAMING);

    while(!snd->isFinished()){
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    snd->drop();
    engine->drop(); // delete engine
}

irrklang::ISoundEngine *SoundTest::startPlay() {
    // start the sound engine with default parameters
    ISoundEngine *engine = createIrrKlangDevice();
    if (!engine) {
        throw runtime_error("Could not start sound engine!");
    }

    // play some sound stream, looped
    engine->play2D(soundFile().string().c_str(), false, false, false, ESM_NO_STREAMING);

    return engine;
}

std::filesystem::path SoundTest::soundFile() {
    return globals::renderData() / "sounds" / "WinXP.ogg";
}
std::filesystem::path SoundTest::songFile() {
    return globals::renderData() / "sounds" / "Duncan Woods" / "Cameleon" / "Wait.mp3.ogg";
}
