#ifndef _PLAYER_H
#define _PLAYER_H

#include "_modelobj.h"
#include "_physicsobject.h"
#include <glm/glm.hpp>

/* Represents the player (rat) in the game.

    Combines:
    - rendering (model)
    - movement (physics)
    - orientation (rotation)
*/
class _player {
public:
    _player();
    ~_player();

    _modelobj model;          // Handles loading and drawing the OBJ model
    _physicsobject physics;   // Handles position, velocity, and movement
    glm::vec3 rot;            // Stores rotation (mainly Y for facing direction)

    void init(const std::string& modelPath, float scale); // Loads the player model
    void draw(); // Renders the player in the scene
};

#endif
