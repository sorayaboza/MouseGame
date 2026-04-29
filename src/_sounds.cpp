#include "_sounds.h"

_sounds::_sounds()
{
    //ctor
}

_sounds::~_sounds()
{
    //dtor
    sndEng->drop();
}
void _sounds::playMusic(char* fileName)
{
    sndEng->play2D(fileName,true);
    //music play on a loop
}

void _sounds::playSounds(char* fileName)
{
  // if(!sndEng->isCurrentPlaying(fileName))
   sndEng->play2D(fileName,false,false);
}

void _sounds::pauseSounds(char* fileName)
{
   sndEng->play2D(fileName,true,false);
}

void _sounds::iniSounds()
{
    if(!sndEng)
        cout<<"ERROR:  **Sound Enging could not start"<<endl;
}
