#include "_camera.h"

_camera::_camera() { //ctor
    // {eye.x, eye.y, eye.z}
    eye = {0,50,0}; // eye.z sets the camera up and down
    des = {0,0,0};
    up = {0,0,-1};
    step = 0.5;
    theta = {0,0};
    distance = sqrt(pow(eye.x-des.x,2)+pow(eye.y-des.y,2)+pow(eye.z-des.z,2));
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
