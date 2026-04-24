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
    texture.loadTexture((char*)"images/mouse.png"); // Load rat texture
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
            // ----------- DRAW BLOB SHADOW -----------
        glPushMatrix();
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);

            // Position shadow on the ground (slightly above to prevent z-fighting)
            glTranslatef(physics.pos.x, physics.pos.y - 0.9f, physics.pos.z);

            // Flatten into ellipse
            float height = physics.pos.y; // how high player is
            float scale = 1.5f + (height * 0.1f); // tweak multiplier

            glScalef(scale, 1.0f, scale);

            // Draw circular shadow
            glBegin(GL_TRIANGLE_FAN);
                // Center (darker)
                glColor4f(0.05f, 0.15f, 0.6f, 0.25f);
                glVertex3f(0.0f, 0.0f, 0.0f);

                int segments = 32;
                float radius = 0.5f;

                for (int i = 0; i <= segments; i++) {
                    float angle = (2.0f * 3.14159f * i) / segments;
                    float x = cos(angle) * radius;
                    float z = sin(angle) * radius;

                    // Edge (fades out)
                    glColor4f(0.2f, 0.0f, 0.4f, 0.0f);
                    glVertex3f(x, 0.0f, z);
                }
            glEnd();

            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
        glPopMatrix();
        // ----------- END SHADOW -----------

        texture.BindTex();                 // bind player texture
        // Move player to its position in the world
        glTranslatef(physics.pos.x, physics.pos.y, physics.pos.z);
        glRotatef(rot.y, 0, 1, 0); // Rotate player around Y-axis (facing direction)

        glEnable(GL_TEXTURE_2D);           // ensure textures enabled
        glColor3f(1.0f, 1.0f, 1.0f);       // prevent tinting

        model.drawModel();   // draw model with UVs

        glBindTexture(GL_TEXTURE_2D, 0); // reset texture
    glPopMatrix(); // Restore previous transformation state
}
