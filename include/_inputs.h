#ifndef _INPUTS_H
#define _INPUTS_H

#include<_common.h>
#include<_skybox.h>
#include<_camera.h>
#include "_player.h"

class _inputs
{
    public:
        _inputs();
        virtual ~_inputs();

        void keyPressed(_player* player);
        void keyUp();

        bool isTranslate;
        bool isRotate;

        double prev_mX;
        double prev_mY;

        WPARAM wParam;

    protected:

    private:
};

#endif // _INPUTS_H
