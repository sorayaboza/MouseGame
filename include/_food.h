#ifndef _FOOD_H
#define _FOOD_H

#include "_physicsobject.h"
#include "_modelobj.h"
#include <string>

/*
    Represents a food item in the game.

    Combines:
    - physics object for motion
    - OBJ model for rendering
*/
class _food {
public:
    _food(const std::string& modelPath, float scale = 1.0f);
    ~_food(); // destructor

    _physicsobject physics; // physics for position/velocity
    _modelobj model;        // 3D model

    float modelScale;       // scale of the visual model
    float collisionRadius;  // radius used for collision detection

    void update(float dt, float floorY);  // update physics
    void draw();                          // draw model
};

#endif // _FOOD_H
