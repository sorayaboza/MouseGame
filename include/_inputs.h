#ifndef _INPUTS_H
#define _INPUTS_H

#include<_common.h>
#include<_model.h>
#include<_skybox.h>
#include<_camera.h>

class _inputs
{
    public:
        _inputs();
        virtual ~_inputs();

        void keyPressed(_model*);
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
