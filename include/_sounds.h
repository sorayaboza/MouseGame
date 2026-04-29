#ifndef _SOUNDS_H
#define _SOUNDS_H

#include<SNDS/irrKlang.h>
#include<_common.h>

using namespace irrklang;

class _sounds
{
    public:
        _sounds();
        virtual ~_sounds();

        void playMusic(char*);
        void playSounds(char*);
        void pauseSounds(char*);
        void iniSounds();

        ISoundEngine *sndEng = createIrrKlangDevice();

    protected:

    private:
};

#endif // _SOUNDS_H
