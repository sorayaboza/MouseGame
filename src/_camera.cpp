/*
_camera.cpp

PURPOSE:
Controls the game camera.

This file:
- Stores camera position
- Stores camera target
- Moves the camera
- Sets up the OpenGL view using gluLookAt()
*/

#include "_camera.h"

// Constructor
// Initializes camera position and settings.
_camera::_camera() {

    // Camera position
    eye = {0, 30, 30}; // Slightly above and tilted toward the scene
    des = {0, 0, 0};   // Position the camera looks at
    up = {0, 1, 0};    // Up direction for the camera
    step = 0.5;        // Camera movement step size
    theta = {0,0};     // Rotation values
    distance = 35.0f;  // Distance from target
}


/*
Destructor
*/
_camera::~_camera() { /*dtor*/ }

void _camera::camReset(){ }
void _camera::camRotX() { }
void _camera::camRotY() { }


/*
Moves camera forward/backward.

dir:
    -1 = backward
     1 = forward
*/
void _camera::camMoveFB(int dir) {
    eye.z += step * dir; // Move camera position
    des.z += step * dir; // Move look-at target
}


/*
Moves camera left/right.

dir:
    -1 = left
     1 = right
*/
void _camera::camMoveLR(int dir) {
    eye.x += step * dir; // Move camera position
    des.x += step * dir; // Move look-at target
}


// Sets the OpenGL camera view.
void _camera::setUpCamera() {
    /*
    gluLookAt(
        eye position,
        target position,
        up direction
    );
    */
    gluLookAt(
        eye.x, eye.y, eye.z,
        des.x, des.y, des.z,
        up.x,  up.y,  up.z
    );
}
