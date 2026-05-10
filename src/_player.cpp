#include "_player.h"
#include <GL/glew.h>

// PURPOSE: Handles basic player model and animation state.

// Initializes player variables.
_player::_player() {
    model    = new _ModelLoaderMD2(); // Create MD2 model loader
    isMoving = false; // Player starts idle
    animTime = 0.0f; // Animation timer
}
_player::~_player() { /* model is managed by scene */ }

// Loads the player model and texture.
void _player::init(const std::string& modelPath, const std::string& texturePath) {
    model->initModel(modelPath.c_str(), (char*)texturePath.c_str());
}

// Updates animation timing while moving.
void _player::update(float dt) {
    if (isMoving) animTime += dt; // Increase animation timer while player moves
}
