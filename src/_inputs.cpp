/*
================================================
_inputs.cpp

PURPOSE:
Handles keyboard movement input for the player.

This file:
- Detects WASD keys
- Moves the player
- Rotates the player toward movement direction
- Prevents faster diagonal movement

================================================
*/

#include "_inputs.h"

// Constructor
// Initializes input state variables.
_inputs::_inputs() {
     isRotate = false;      // Rotation mode off
     isTranslate = false;   // Translation mode off

     prev_mX = prev_mY = 0; // Previous mouse position
}

_inputs::~_inputs() { /*dtor*/ }


// Handles WASD movement input.
void _inputs::keyPressed(_player* player) {
    float moveSpeed = 11.0f; // Player movement speed

    // Movement direction values
    float dx = 0;
    float dz = 0;

    // KEY INPUT
    if (wParam == 65) dx -= 1;  // A = left
    if (wParam == 68) dx += 1;  // D = right
    if (wParam == 87) dz -= 1;  // W = forward
    if (wParam == 83) dz += 1;  // S = backward

    // Normalize movement vector.
    // Prevents diagonal movement from being faster.
    float len = sqrt(dx*dx + dz*dz);


    // Only move if a movement key was pressed
    if (len > 0) {

        // Normalize direction and apply speed
        dx = dx / len * moveSpeed;
        dz = dz / len * moveSpeed;

        // Move player position
        player->physics.pos.x += dx;
        player->physics.pos.z += dz;

        // Rotate player toward movement direction.
        // atan2() returns angle in radians, then converted to degrees.
        player->rot.y =
            atan2(dx, dz) * 180.0f / 3.14159f;
    }
}


// Handles key release events.
void _inputs::keyUp() {

    switch(wParam) {

        default:
            break;
    }
}
