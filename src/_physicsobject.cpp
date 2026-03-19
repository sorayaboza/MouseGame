#include "_physicsobject.h"
#include <cmath>

// Updates the object's motion using simple Euler integration. Applies gravity and handles floor collisions.
_physicsobject::_physicsobject() { //ctor
    pos = {0,0,0};
    velocity = {0,0,0};
    acceleration = {0,-15.0f,0}; // gravity
}

_physicsobject::~_physicsobject() { /*dtor*/ }

void _physicsobject::updatePhysics(float dt) {
    // Update velocity
    velocity.x += acceleration.x * dt;
    velocity.y += acceleration.y * dt;
    velocity.z += acceleration.z * dt;

    // Update position
    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
    pos.z += velocity.z * dt;

    // Floor collision
    if(pos.y <= 0.0f)
    {
        pos.y = 0.0f;         // clamp to floor
        velocity.y *= -0.5f;  // bounce
        if(fabs(velocity.y) < 0.01f)
            velocity.y = 0.0f; // stop tiny oscillations
    }
}
