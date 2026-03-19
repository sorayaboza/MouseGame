#include "_inputs.h"

_inputs::_inputs() { //ctor
     isRotate = isTranslate = false;
     prev_mX = prev_mY = 0;
}

_inputs::~_inputs() { /*dtor*/ }

void _inputs::keyPressed(_model* mdl) {
    float moveSpeed = 8.0f;

    // movement vector
    float dx = 0, dz = 0;
    if (wParam == 65) dx -= 1;  // A
    if (wParam == 68) dx += 1;  // D
    if (wParam == 87) dz -= 1;  // W
    if (wParam == 83) dz += 1;  // S

    // normalize to prevent diagonal speed boost
    float len = sqrt(dx*dx + dz*dz);
    if (len > 0) {
        dx = dx / len * moveSpeed;
        dz = dz / len * moveSpeed;

        mdl->pos.x += dx;
        mdl->pos.z += dz;
    }
}

void _inputs::keyUp() { switch(wParam) { default: break; } }
