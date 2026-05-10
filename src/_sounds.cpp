#include "_sounds.h"
#include <map>
#include <chrono>
#include <string>

_sounds::_sounds()
    : currentMusic(nullptr), currentMusicFile(),
      footstepLoop(nullptr)
{
}

_sounds::~_sounds()
{
    if (currentMusic) {
        currentMusic->drop();
        currentMusic = nullptr;
    }
    if (footstepLoop) {
        footstepLoop->stop();
        footstepLoop->drop();
        footstepLoop = nullptr;
    }
    sndEng->drop();
}

// ── Background music – swap to new track if different ─────────────
void _sounds::playMusic(char* fileName)
{
    if (!fileName) return;
    std::string fn(fileName);

    // Already playing this exact track?  Don't restart.
    if (currentMusicFile == fn && currentMusic && !currentMusic->isFinished())
        return;

    // Stop any previously-playing music
    stopMusic();

    // Start the new track on a loop, hold the handle so we can stop later.
    // Volume is set to 0.45 so SFX sit clearly above the music.
    currentMusic = sndEng->play2D(fileName, true, false, true);
    if (currentMusic)
        currentMusic->setVolume(0.45f);
    currentMusicFile = fn;
}

void _sounds::stopMusic()
{
    if (currentMusic) {
        currentMusic->stop();
        currentMusic->drop();
        currentMusic = nullptr;
    }
    currentMusicFile.clear();
}

// ── One-shot SFX (full volume = 1.0 by default, louder than music) ─
void _sounds::playSounds(char* fileName)
{
    sndEng->play2D(fileName, false, false);
}

// ── Throttled SFX – minimum gap between identical triggers ────────
void _sounds::playSfxThrottled(const char* fileName, float minIntervalSec)
{
    static std::map<std::string, double> lastPlayed;

    using clk = std::chrono::steady_clock;
    double now = std::chrono::duration<double>(clk::now().time_since_epoch()).count();

    auto it = lastPlayed.find(fileName);
    if (it != lastPlayed.end() && (now - it->second) < minIntervalSec) return;

    lastPlayed[fileName] = now;
    sndEng->play2D(fileName, false, false);
}

// ── Looping footsteps (or any other sustained loop) ──
// playLoop() starts the file looping if it isn't already playing it.
// stopLoop() halts it cleanly so leftover voices don't ring out.
void _sounds::playLoop(const char* fileName)
{
    if (footstepLoop && !footstepLoop->isFinished()) return;   // already running
    if (footstepLoop) {
        footstepLoop->drop();
        footstepLoop = nullptr;
    }
    footstepLoop = sndEng->play2D(fileName, true, false, true);
    if (footstepLoop)
        footstepLoop->setVolume(0.85f);   // a bit under SFX so it's not piercing
}

void _sounds::stopLoop()
{
    if (footstepLoop) {
        footstepLoop->stop();
        footstepLoop->drop();
        footstepLoop = nullptr;
    }
}

void _sounds::pauseSounds(char* fileName)
{
    sndEng->play2D(fileName, true, false);
}

void _sounds::iniSounds()
{
    if (!sndEng)
        cout << "ERROR:  **Sound Engine could not start" << endl;
}
