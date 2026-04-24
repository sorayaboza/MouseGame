#ifndef _FOOD_H
#define _FOOD_H

#include "_physicsobject.h"
#include "_modelobj.h"
#include <string>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <_texloader.h>

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
    _texLoader texture;     // 3d model texture

    float modelScale;       // scale of the visual model
    float collisionRadius;  // radius used for collision detection

    void update(float dt, float floorY);  // update physics
    void draw(float floorY);                          // draw model

    glm::vec3 rot;
};

#endif // _FOOD_H
