#include "_food.h"
#include <GL/glew.h>

// Constructor for food object. Loads the model and prepares it for rendering.
_food::_food(const std::string& modelPath, float scale) { //ctor
    model.loadOBJ(modelPath, scale);
}

_food::~_food() { /*dtor*/ }

// Updates the food's physics each frame
void _food::update(float dt, float floorY) {
    const float gravity = -9.8f; // units per second^2

    // Apply gravity
    physics.velocity.y += gravity * dt;

    // Update position
    physics.pos += physics.velocity * dt;

    // Simple floor collision
    if (physics.pos.y < floorY + collisionRadius) {
        physics.pos.y = floorY + collisionRadius;
        physics.velocity.y *= -0.5f; // bounce with damping
    }
}

// Draws the food at its current position
void _food::draw() {
    glPushMatrix();
    glTranslatef(physics.pos.x, physics.pos.y, physics.pos.z);
    model.drawModel();
    glPopMatrix();
}
