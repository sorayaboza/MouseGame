#include "_food.h"
#include <GL/glew.h>

// Constructor for food object. Loads the model and prepares it for rendering.
_food::_food(const std::string& modelPath, float scale) { //ctor
    model.loadOBJ(modelPath, scale);
    texture.loadTexture((char*)"images/default.png"); // Load a texture (example)
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
        glEnable(GL_TEXTURE_2D); // enable textures
        glColor3f(1.0f, 1.0f, 1.0f);

        texture.BindTex();   // bind texture before drawing

        glTranslatef(physics.pos.x, physics.pos.y, physics.pos.z);
        glRotatef(rot.y, 0, 1, 0);

        model.drawModel(); // draw model with UVCs

        glBindTexture(GL_TEXTURE_2D, 0); // reset after drawing
    glPopMatrix();
}
