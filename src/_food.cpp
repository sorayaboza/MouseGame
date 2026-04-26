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

// Draws the food at its current position
void _food::draw(float floorY) {
    // ----------- DRAW SHADOW (ADD THIS BLOCK) -----------
    glPushMatrix();
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        // Position shadow on ground
        glTranslatef(physics.pos.x, floorY + 0.02f, physics.pos.z);

        float height = physics.pos.y - floorY;
        float scale = 1.2f + (height * 0.08f);
        glScalef(scale, 1.0f, scale);

        glBegin(GL_TRIANGLE_FAN);
            glColor4f(0.05f, 0.15f, 0.6f, 0.25f);
            glVertex3f(0.0f, 0.0f, 0.0f);

            int segments = 24;
            float radius = 0.5f;

            for (int i = 0; i <= segments; i++) {
                float angle = (2.0f * 3.14159f * i) / segments;
                float x = cos(angle) * radius;
                float z = sin(angle) * radius;

                glColor4f(0.05f, 0.15f, 0.6f, 0.0f);
                glVertex3f(x, 0.0f, z);
            }
        glEnd();
    glPopMatrix();
    // ----------- END SHADOW -----------

    glPushMatrix();
        glEnable(GL_TEXTURE_2D); // enable textures
        glColor3f(1.0f, 1.0f, 1.0f);

        glTranslatef(physics.pos.x, physics.pos.y, physics.pos.z);
        glRotatef(-90, 1, 0, 0);

        glScalef(0.1f, 0.1f, 0.1f);

        model->Draw();

        glBindTexture(GL_TEXTURE_2D, 0); // reset after drawing
    glPopMatrix();
}
