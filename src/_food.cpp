#include "_food.h"
#include <GL/glew.h>

// Constructor for food object. Loads the model and prepares it for rendering.
_food::_food(const std::string& modelPath, float scale) { //ctor
    model.loadOBJ(modelPath, scale);
}

_food::~_food() { /*dtor*/ }

// Updates the food's physics each frame
void _food::update(float dt, float floorY) {
    physics.updatePhysics(dt, floorY + collisionRadius);
    rot.y += 90.0f * dt; // spin slowly
}

// Draws the food at its current position
void _food::draw() {
    glPushMatrix();
    glTranslatef(physics.pos.x, physics.pos.y, physics.pos.z);
    glRotatef(rot.y, 0, 1, 0);
    model.drawModel();
    glPopMatrix();
}
