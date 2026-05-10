#ifndef _SOUNDS_H
#define _SOUNDS_H

#include<SNDS/irrKlang.h>
#include<_common.h>
#include<string>

using namespace irrklang;

class _sounds
{
    public:
        _sounds();
        virtual ~_sounds();

        // Background music – only one track plays at a time.
        // Calling playMusic with a new file stops the previous one.
        // Music plays at reduced volume (0.45) so SFX sit above it.
        void playMusic(char*);
        void stopMusic();

        // Generic one-shot SFX (no overlap protection, full volume)
        void playSounds(char*);

        // SFX with built-in spam protection: will not retrigger more
        // often than minIntervalSec (in seconds).
        void playSfxThrottled(const char* fileName, float minIntervalSec);

        // Looping SFX (e.g. enemy footsteps).  Only ONE loop at a time.
        // playLoop starts it if not already playing; stopLoop halts cleanly.
        void playLoop(const char* fileName);
        void stopLoop();

        void pauseSounds(char*);
        void iniSounds();

        ISoundEngine *sndEng = createIrrKlangDevice();

    protected:
        ISound*       currentMusic;
        std::string   currentMusicFile;
        ISound*       footstepLoop;       // sustained chase-footsteps loop

    private:
};

#endif // _SOUNDS_H
