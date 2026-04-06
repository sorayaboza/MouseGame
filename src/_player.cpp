#include "_player.h"
#include <GL/glew.h>

/* Represents the player-controlled rat in the game world.

    Uses:
    - _modelobj to load and render the OBJ model
    - _physicsobject to handle movement and collisions
    - rotation vector to control facing direction
*/
_player::_player() { // ctor
    rot = {0,0,0}; // Initialize rotation to zero (facing forward)
}

_player::~_player() { /*dtor*/ }

/* init()

    Loads the player model from an OBJ file.

    Parameters:
    - modelPath: file path to the OBJ model
    - scale: size multiplier applied during model loading

    Steps:
    1. Call model loader to read OBJ file
    2. Apply scaling to fit the scene
*/
void _player::init(const std::string& modelPath, float scale) {
    model.loadOBJ(modelPath, scale);
}

/* draw()

    Renders the player in the world.

    Steps:
    1. Apply translation using physics position
    2. Apply rotation so player faces movement direction
    3. Draw the model using VAO/VBO buffers
*/
void _player::draw() {
    glPushMatrix(); // Save current transformation state

    // Move player to its position in the world
    glTranslatef(physics.pos.x, physics.pos.y, physics.pos.z);

    glRotatef(rot.y, 0, 1, 0); // Rotate player around Y-axis (facing direction)
    model.drawModel(); // Draw the OBJ model

    glPopMatrix(); // Restore previous transformation state
}
