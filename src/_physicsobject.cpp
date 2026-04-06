#include "_physicsobject.h"
#include <cmath>

// Updates the object's motion using simple Euler integration. Applies gravity and handles floor collisions.
_physicsobject::_physicsobject() { //ctor
    pos = {0,0,0};
    velocity = {0,0,0};
    acceleration = {0,-15.0f,0}; // gravity
}

_physicsobject::~_physicsobject() { /*dtor*/ }

void _physicsobject::updatePhysics(float dt, float floorY) {
    // Update velocity and position
    velocity += acceleration * dt;
    pos += velocity * dt;

    // Floor collision
    if (pos.y <= floorY) {
        pos.y = floorY;
        velocity.y *= -0.5f;

        if (fabs(velocity.y) < 0.01f)
            velocity.y = 0.0f;
    }
}
