#include "_camera.h"

_camera::_camera() {
    // Slight tilt instead of straight top-down
    eye = {0, 30, 30};   // tilted toward +Z

    des = {0, 0, 0};     // look at center

    up = {0, 1, 0};      // correct up direction

    step = 0.5;
    theta = {0,0};

    distance = 35.0f;
}

_camera::~_camera() { /*dtor*/ }
void _camera::camReset(){ }

void _camera::camRotX() { }

void _camera::camRotY() { }

void _camera::camMoveFB(int dir) {
    eye.z += step*dir; //left  dir= -1 and right is dir=1
    des.z += step*dir; //left  dir= -1 and right is dir=1
}

void _camera::camMoveLR(int dir) {
    eye.x += step*dir; //left  dir= -1 and right is dir=1
    des.x += step*dir; //left  dir= -1 and right is dir=1
}

void _camera::setUpCamera() {
    gluLookAt(eye.x,eye.y,eye.z,
              des.x,des.y,des.z,
              up.x,up.y,up.z);
}
