#include "_food.h"
#include <GL/glew.h>

// Constructor for food object. Loads the model and prepares it for rendering.
_food::_food(const std::string& modelPath, float scale) { //ctor
    model = new _ModelLoaderMD2();
    model->initModel(modelPath.c_str(), (char*)"images/milk.png"); // Load a texture (example)
}

_food::~_food() { /*dtor*/ }

// Updates the food's physics each frame
void _food::update(float dt, float floorY) {
    physics.updatePhysics(dt, floorY + collisionRadius);
    rot.y += 90.0f * dt; // spin slowly
}
