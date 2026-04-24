#ifndef _PHYSICSOBJECT_H
#define _PHYSICSOBJECT_H
#include <_common.h>

/*
    Represents a physics-enabled object in the world.

    Handles:
    - position, velocity, acceleration
    - simple gravity and floor collision
*/
class _physicsobject {
public:
    _physicsobject();
    ~_physicsobject();

    glm::vec3 pos;        // current position
    glm::vec3 velocity;   // current velocity
    glm::vec3 acceleration; // forces like gravity

    void updatePhysics(float dt, float floorY); // updates position and velocity based on acceleration
};

#endif // _PHYSICSOBJECT_H
