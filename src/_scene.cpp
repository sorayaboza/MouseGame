#include "_scene.h"
#include <vector>
#include <cstdlib> // for rand()
#include <cmath>
#include <windows.h>

float slideFriction = 0.99f; // Friction of the objects
glm::vec3 playerMoveDir;
int score = 0;
float scale = 1.0f; // Scale of objects

/* Main controller for the game world.

    Responsible for:
    - initializing OpenGL
    - creating world objects (sky box, food, player)
    - updating physics
    - handling collisions
    - drawing the scene each frame
*/
_Scene::_Scene() { // ctor
    myLight = new _lighting();
    keyMS = new _inputs();
    myTex = new _texLoader();
    sky = new _skyBox();
    cam = new _camera();
    mouseHoleRadius = 4.0f;
}

_Scene::~_Scene() { /*dtor*/ }

GLint _Scene::initGL() {
    glewInit(); // Initialize GLEW so OpenGL extensions can be used
    glClearColor(0,0,0,1); // Set background clear color (black)
    glEnable(GL_DEPTH_TEST); // Enable depth testing so closer objects render in front

    // Enable lighting system
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_TEXTURE_2D); // Enable texture mapping
    myLight->setLight(GL_LIGHT0); // Configure light properties

    // ---------------- SKYBOX SETUP ----------------
    // Creates a large textured cube surrounding the scene so the player appears to be inside an environment.
    sky->boxInit();
     // Load textures for each face of the cube
    sky->myTex[0].loadTexture("images/side.png");
    sky->myTex[1].loadTexture("images/side_hole.png");
    sky->myTex[2].loadTexture("images/floor.png");
    sky->myTex[3].loadTexture("images/floor.png");
    sky->myTex[4].loadTexture("images/side.png");
    sky->myTex[5].loadTexture("images/side.png");

    // ---------------- SPAWN FOODS ----------------
    // Create several food objects that fall and bounce inside the sky box.
    int numFoods = 10;      // number of foods to spawn

    // List of possible food models
    struct FoodType {
        std::string model;
        std::string texture;
        float scale;
    };

    std::vector<FoodType> foodTypes = {
        {"models/cheese.obj", "images/cheese.png", 0.8f},
        {"models/donut.obj",  "images/donut.png",  1.2f}
    };

    // Determine playable boundaries from sky box size
    float skyBottomY = sky->pos.y - sky->scale.y * 0.5f; // -35
    float skyTopY    = sky->pos.y + sky->scale.y * 0.5f; // +35

    float floorY = skyBottomY + 1.0f;  // 1 unit above bottom
    mouseHolePos.y = floorY;           // hole sits exactly on floor

    float spawnHeight = floorY + 20.0f;  // 20 units above floor

    float halfX = sky->scale.x * 0.5f; // full half-width
    float halfZ = sky->scale.z * 0.5f; // full half-depth

    for(int i=0; i<numFoods; i++) {
        FoodType type = foodTypes[rand() % foodTypes.size()];
        _food* newFood = new _food(type.model, type.scale);
        newFood->texture.loadTexture((char*)type.texture.c_str()); // Load correct texture

        newFood->physics.pos.x = ((rand()%100)/100.0f)*(2*halfX)-halfX; // Random X position inside sky box bounds
        newFood->physics.pos.z = ((rand()%100)/100.0f)*(2*halfZ)-halfZ; // Random Z position inside sky box bounds

        float spawnHeight = floorY + 40.0f;  // spawn 20 units above the floor
        newFood->physics.pos.y = spawnHeight;

        foods.push_back(newFood); // Store food in vector so it can be updated/drawn later
    }

    // ---------- INITIALIZE PLAYER MODEL ----------
    // Load the rat OBJ model that the player controls.
    player = new _player();
    player->init("models/player.obj", 1.0f);
    player->physics.pos = {0, floorY, 0};

    // --- SETTING WHERE THE HOLE IS ---
    // Bottom-left corner of the back face
    mouseHolePos.x = -halfX + 21.0f;
    mouseHolePos.y = floorY;
    mouseHolePos.z = -halfZ; // back face


    return true;
}

void _Scene::reSize(GLint width, GLint height) {
    GLfloat aspectRatio = (GLfloat)width/(GLfloat)height; // Calculate aspect ratio for proper perspective projection
    glViewport(0,0,width,height); // Adjust OpenGL view port to match new window size

     // Switch to projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Create perspective projection
    gluPerspective(60,aspectRatio,0.1,2000); // (FOV = 60°, near plane = 0.1, far plane = 2000)

    // Return to model view matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Detects collision with food and mouse hole.
void _Scene::checkFoodInHole() {
    for (size_t i = 0; i < foods.size(); /* no i++ here */) {
        _food* food = foods[i];

        // Compute vector from hole center to food
        float dx = food->physics.pos.x - mouseHolePos.x;
        //float dy = food->physics.pos.y - mouseHolePos.y;
        float dz = food->physics.pos.z - mouseHolePos.z;

        float dist = sqrt(dx*dx + dz*dz);
        // float dist = sqrt(dx*dx + dy*dy + dz*dz); // with y version

        // If food is inside hole
        if (dist < mouseHoleRadius) {
            // Food has entered the hole!
            foods.erase(foods.begin() + i); // 1. Remove from the foods vector
            delete food; // 2. Delete the food object
            score += 1; // 3. Increment score or trigger event
            cout << "SCORE: " << score << endl;

            // Do NOT increment i since we erased an element
            continue;
        }

        i++; // Only increment if food wasn't removed
    }
}


void _Scene::drawScene() {
    // Clear color and depth buffers for new frame
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glm::vec3 offset = {
        cam->eye.x - cam->des.x,
        cam->eye.y - cam->des.y,
        cam->eye.z - cam->des.z
    };

    // Move camera to follow player (isometric style)
    cam->des.x = player->physics.pos.x;
    cam->des.y = player->physics.pos.y;
    cam->des.z = player->physics.pos.z;

    float distance = 40.0f; // how far camera stays from player

    float radYaw = glm::radians(cam->yaw);
    float radPitch = glm::radians(cam->pitch);

    // Orbit around player
    cam->eye.x = cam->des.x + distance * cos(radPitch) * sin(radYaw);
    cam->eye.y = cam->des.y + distance * sin(radPitch);
    cam->eye.z = cam->des.z + distance * cos(radPitch) * cos(radYaw);

    cam->setUpCamera();  // Set camera position and orientation
    sky->drawBox(); // Draw sky box

    float floorY = (sky->pos.y - sky->scale.y * 0.5f) + 1;

    // Update physics and render/draw each food
    for (auto& food : foods) {
        food->update(0.016f, floorY); // pass floor
        food->physics.velocity.x *= slideFriction;
        food->physics.velocity.z *= slideFriction;
        food->draw();
    }

    updatePlayer(0.016f); // Assume ~60 fps
    handleCollisions();       // Food-to-food collisions
    handlePlayerCollisions(); // Food-to-player collisions
    checkFoodInHole();
    player->draw(); // Draw the rat model

    // --- DEBUG: Draw mouse hole marker ---
    glPushMatrix();
        glDisable(GL_TEXTURE_2D);
        glTranslatef(mouseHolePos.x, mouseHolePos.y, mouseHolePos.z); // Move to hole position
        glColor3f(1.0f, 0.0f, 0.0f); // Red cube
        glutSolidCube(1.0f);          // 1 unit cube for visibility
        glEnable(GL_TEXTURE_2D);
    glPopMatrix();

}

void _Scene::updatePlayer(float dt) {
    float speed = 20.0f * dt; // Player movement speed scaled by frame time

    glm::vec3 moveVec(0.0f);

    // Camera forward (from eye --> target)
    glm::vec3 camForward = glm::normalize(glm::vec3(
        cam->des.x - cam->eye.x,
        0.0f, // ignore vertical
        cam->des.z - cam->eye.z
    ));

    // Camera right (perpendicular)
    glm::vec3 camRight = glm::normalize(glm::cross(camForward, glm::vec3(0,1,0)));

    // WASD relative to camera
    if (GetAsyncKeyState('W') & 0x8000) moveVec += camForward;
    if (GetAsyncKeyState('S') & 0x8000) moveVec -= camForward;
    if (GetAsyncKeyState('A') & 0x8000) moveVec -= camRight;
    if (GetAsyncKeyState('D') & 0x8000) moveVec += camRight;

    // Save movement direction for pushing
    if (glm::length(moveVec) > 0.0f) {
        playerMoveDir = glm::normalize(moveVec);
    }

    // Normalize so diagonal isn't faster
    if (glm::length(moveVec) > 0.0f)
        moveVec = glm::normalize(moveVec) * speed;

    // Apply movement
    player->physics.pos += moveVec;

    // Rotate the rat so it faces the direction it moves
    if (glm::length(moveVec) > 0.0f) {
        player->rot.y = glm::degrees(atan2(moveVec.x, moveVec.z));
    }

    // Sky box boundary: Prevents player from leaving the skybox
    float halfX = sky->scale.x * 0.47f;
    float halfZ = sky->scale.z * 0.47f;

    if (player->physics.pos.x < -halfX) player->physics.pos.x = -halfX;
    if (player->physics.pos.x > halfX)  player->physics.pos.x = halfX;
    if (player->physics.pos.z < -halfZ) player->physics.pos.z = -halfZ;
    if (player->physics.pos.z > halfZ)  player->physics.pos.z = halfZ;
}

/* handleCollisions()

    Detects collisions between food objects using sphere collision.

    Steps:
    1. Compute distance between food centers.
    2. If distance < combined radius, collision occurred.
    3. Push the objects apart along the collision normal.
    4. Apply a collision impulse so momentum transfers between foods.
       This causes both objects to react naturally when they collide.
*/
void _Scene::handleCollisions() {

    float radius = 2.0f; // collision radius for each food

    // Compare every pair of foods
    for (size_t i = 0; i < foods.size(); i++) {
        for (size_t j = i + 1; j < foods.size(); j++) {

            _food* a = foods[i];
            _food* b = foods[j];

            // Vector from food B to food A
            float dx = a->physics.pos.x - b->physics.pos.x;
            float dy = a->physics.pos.y - b->physics.pos.y;
            float dz = a->physics.pos.z - b->physics.pos.z;

            // Distance between food centers
            float dist = sqrt(dx*dx + dy*dy + dz*dz);

            // Minimum distance before collision occurs
            float minDist = radius * 2.0f;

            // Check if foods are intersecting
            if (dist < minDist && dist > 0.0001f) {

                // Normalize the collision direction (collision normal)
                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;

                // Amount the objects overlap
                float overlap = minDist - dist;

                // Push objects apart equally so they no longer intersect
                a->physics.pos.x += nx * overlap * 0.5f;
                a->physics.pos.y += ny * overlap * 0.5f;
                a->physics.pos.z += nz * overlap * 0.5f;

                b->physics.pos.x -= nx * overlap * 0.5f;
                b->physics.pos.y -= ny * overlap * 0.5f;
                b->physics.pos.z -= nz * overlap * 0.5f;

                // --- Collision Physics ---

                // Relative velocity between the two foods
                float rvx = a->physics.velocity.x - b->physics.velocity.x;
                float rvy = a->physics.velocity.y - b->physics.velocity.y;
                float rvz = a->physics.velocity.z - b->physics.velocity.z;

                // Velocity along the collision normal
                float velAlongNormal = rvx*nx + rvy*ny + rvz*nz;

                // If objects are moving away from each other, skip resolution
                if (velAlongNormal > 0)
                    continue;

                // Coefficient of restitution (bounciness)
                float bounce = 1.0f;

                // Calculate collision impulse
                float impulse = -(1 + bounce) * velAlongNormal;

                // Divide impulse for equal-mass objects
                impulse *= 0.5f;

                // Apply impulse to both foods (equal and opposite forces)
                a->physics.velocity.x += impulse * nx;
                a->physics.velocity.y += impulse * ny;
                a->physics.velocity.z += impulse * nz;

                b->physics.velocity.x -= impulse * nx;
                b->physics.velocity.y -= impulse * ny;
                b->physics.velocity.z -= impulse * nz;
            }
        }
    }
}


// Detects collisions between foods and the player.
void _Scene::handlePlayerCollisions() {
    float halfSize = 2.0f; // half cube size for player & food
    float halfX = sky->scale.x * 0.50f;  // skybox X boundary
    float halfZ = sky->scale.z * 0.50f;  // skybox Z boundary
    float foodRadius = 2.0f;   // size of the food collision cube/sphere
    float radius = 2.0f; // collision radius

    for (auto& food : foods) {

        // Vector from player to food
        float dx = food->physics.pos.x - player->physics.pos.x;
        float dy = 0.0f; // Keeps food ground-based
        float dz = food->physics.pos.z - player->physics.pos.z;

        float dist = sqrt(dx*dx + dy*dy + dz*dz);
        float minDist = radius * 2.0f;

        if (dist < minDist && dist > 0.0001f) {
            // Normalize push direction
            float nx = dx / dist;
            float ny = dy / dist;
            float nz = dz / dist;

            // Separate the objects
            float overlap = minDist - dist;

            food->physics.pos.x += nx * overlap;
            food->physics.pos.y += ny * overlap;
            food->physics.pos.z += nz * overlap;

            // Apply push velocity from the player
            float pushStrength = 6.0f;

            food->physics.velocity.x += playerMoveDir.x * pushStrength;
            food->physics.velocity.z += playerMoveDir.z * pushStrength;
        }

        // Sky box boundary: Keeps food inside the world
        if (food->physics.pos.x - foodRadius < -halfX) {
            food->physics.pos.x = -halfX + foodRadius;
            food->physics.velocity.x *= -0.9f;
        }
        if (food->physics.pos.x + foodRadius > halfX) {
            food->physics.pos.x = halfX - foodRadius;
            food->physics.velocity.x *= -0.9f;
        }
        if (food->physics.pos.z - foodRadius < -halfZ) {
            food->physics.pos.z = -halfZ + foodRadius;
            food->physics.velocity.z *= -0.9f;
        }
        if (food->physics.pos.z + foodRadius > halfZ) {
            food->physics.pos.z = halfZ - foodRadius;
            food->physics.velocity.z *= -0.9f;
        }
    }
}

void _Scene::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {

    case WM_RBUTTONDOWN:
    {
        cam->isRightMouseDown = true;

        // Hide cursor
        ShowCursor(FALSE);

        // Lock cursor to window
        RECT rect;
        GetClientRect(hWnd, &rect);

        POINT ul = {rect.left, rect.top};
        POINT lr = {rect.right, rect.bottom};

        ClientToScreen(hWnd, &ul);
        ClientToScreen(hWnd, &lr);

        rect.left = ul.x;
        rect.top = ul.y;
        rect.right = lr.x;
        rect.bottom = lr.y;

        ClipCursor(&rect);

        // Center cursor
        int centerX = (rect.left + rect.right) / 2;
        int centerY = (rect.top + rect.bottom) / 2;
        SetCursorPos(centerX, centerY);

        cam->lastMouseX = centerX;
        cam->lastMouseY = centerY;
    }
    break;

    case WM_RBUTTONUP:
        cam->isRightMouseDown = false;

        // Show cursor again
        ShowCursor(TRUE);

        // Release cursor lock
        ClipCursor(NULL);
    break;

    case WM_MOUSEMOVE:
    {
        if (cam->isRightMouseDown) {

            POINT p;
            GetCursorPos(&p);

            int dx = p.x - cam->lastMouseX;
            int dy = p.y - cam->lastMouseY;

            cam->yaw   -= dx * 0.3f; // (your fixed direction)
            cam->pitch += dy * 0.3f;

            // Clamp pitch
            if (cam->pitch > 80.0f) cam->pitch = 80.0f;
            if (cam->pitch < 5.0f)  cam->pitch = 5.0f;

            // Re-center cursor EVERY FRAME
            SetCursorPos(cam->lastMouseX, cam->lastMouseY);
        }
    }
    break;
    }
}
