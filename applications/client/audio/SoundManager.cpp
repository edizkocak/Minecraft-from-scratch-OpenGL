#include "SoundManager.h"
#include "globals.h"

using namespace std;
namespace fs = std::filesystem;
using namespace irrklang;

SoundManager::SoundManager() {
    // start the sound engine
    engine = createIrrKlangDevice();
    if (!engine) {
        throw runtime_error("Could not start sound engine!");
    }

    musicRelaxedPath = globals::renderData() / "sounds" / "Duncan Woods" / "Cameleon" / "merged.ogg";

    // play some sound stream, looped
    music = engine->play2D(
            musicRelaxedPath.string().c_str(),
            true, false, true, ESM_NO_STREAMING);
    if(!music){
        throw runtime_error("Could not load music!");
    }
}

SoundManager::~SoundManager() {
    if(music){
        music->drop();
    }
    engine->drop(); // delete engine
}
