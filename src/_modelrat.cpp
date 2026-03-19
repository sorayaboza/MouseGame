#include "_modelRat.h"
#include <iostream>
#include <GL/glew.h>

/*
    Loads and renders the player model from an OBJ file.

    Uses:
    - Inherits all OBJ loading and buffer setup from _modelObj
    - Applies player-specific position, rotation, and velocity
    - Draws using OpenGL VAOs created in the base class
*/

_modelRat::_modelRat(const std::string& filename, float scale) { // ctor
    // Initialize transform and physics values
    pos = {0,0,0};
    rot = {0,0,0};
    velocity = {0,0,0};

    // Attempt to load the OBJ model using base class function
    if (!this->loadOBJ(filename, modelScale)) {
        std::cerr << "Failed to load player OBJ: " << filename << std::endl;
    }
}

_modelRat::~_modelRat() { /*dtor*/ }

void _modelRat::drawModel() {
    if (!buffersInitialized) return;

    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z); // use _modelRat pos
    glRotatef(rot.y, 0, 1, 0);        // use _modelRat rot

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glPopMatrix();
}
