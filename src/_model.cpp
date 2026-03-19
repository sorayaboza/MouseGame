#include "_model.h"

/*
    Represents a physics object in the world.

    In this project, _model is used for the melons. Each melon behaves like a sphere that:
    - falls due to gravity
    - collides with the floor
    - bounces with reduced energy
*/

_model::_model() { //ctor
    quad = gluNewQuadric(); // Create quadric object once (used to draw spheres)

    pos = {0,0,0};
    velocity = {0,0,0};
    acceleration = {0,-0.01f,0}; // 0.01f: Constant downward acceleration (gravity)
}

_model::~_model() { /*dtor*/ }

/* updatePhysics()

    Updates the object's vertical motion using simple physics.

    Steps:
    1. Apply gravity to velocity
    2. Update position using velocity
    3. Check collision with the floor
    4. Bounce with reduced energy
*/
void _model::updatePhysics() {
    velocity.y += acceleration.y; // Apply gravity to vertical velocity
    pos.y += velocity.y; // Move object based on velocity

    // Handling floor collions
    if(pos.y <= 0.0f) { // If the object reaches the floor (y = 0), then...
        pos.y = 0.0f; // Clamp object to the floor
        velocity.y *= -0.5f; // Reverse velocity to simulate bounce. Multiply by -0.5 to lose energy
    }
}

/* drawModel()

    Renders the melon sphere.

    Steps:
    1. Enable texture mapping
    2. Apply object transformations
       (translation, rotation, scale)
    3. Draw a textured sphere using GLU
*/
void _model::drawModel() {
    gluQuadricTexture(quad, GL_TRUE); // Generate texture coordinates
    glEnable(GL_TEXTURE_2D); // Enable texture mapping
    myTex->BindTex(); // Bind melon texture

    glPushMatrix(); // Save current transformation state
        glColor3f(1.0,1.0,1.0);  // Set object color (white so texture is not tinted)
        glTranslatef(pos.x,pos.y,pos.z);  // Move object to its position in the world

        // Apply rotations
        glRotatef(rot.x, 1,0,0);
        glRotatef(rot.y, 0,1,0);
        glRotatef(rot.z, 0,0,1);

        glScalef(scale.x,scale.y,scale.z); // Apply scaling

        // Draw the sphere (melon). Radius = 2.0. 40 slices/stacks make it smooth
        gluSphere(quad, 2.0, 40, 40);
    glPopMatrix(); // Restore previous transformation state
}

// Initializes the object's transform and loads its texture.
void _model::initModel(char * filename) {
    rot = {0,0,0}; // Reset rotation
    pos = {0.0,0.0,-8}; // Set starting position
    scale = {1.0,1.0,1.0}; // Default scale

    myTex->loadTexture(filename); // Load texture for the model
}
