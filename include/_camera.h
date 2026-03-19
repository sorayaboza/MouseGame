#ifndef _CAMERA_H
#define _CAMERA_H

#include<_common.h>

class _camera
{
    public:
        _camera();
        virtual ~_camera();

        vec3 eye; // camera position
        vec3 des; // Where you are looking at
        vec3 up;  // Camera head orientation

        float step; //camera speed
        vec2 theta; // Rotation Angle
        float distance; //eye to des

        void camReset(); // Reset to original settings
        void camRotX();  // rotate camera around X axis
        void camRotY();  // rotate camera around y axis

        void camMoveFB(int);// Move Camera front and back
        void camMoveLR(int);// Move camera to sides

        void setUpCamera(); // call setting in the display

    protected:

    private:
};

#endif // _CAMERA_H
