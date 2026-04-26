#include "_scene.h"
#include <vector>
#include <cstdlib> // for rand()
#include <cmath>
#include <windows.h>

#include <glm/gtc/matrix_transform.hpp>

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
    foodSystem = new _foodsystem();
    ui = new _ui();
    abilities = new _abilities();
    renderer = new _renderer();

    mouseHoleRadius = 6.0f;
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
    glEnable(GL_BLEND); // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_NORMALIZE); // Ensures all normals are unit length
    myLight->setLight(GL_LIGHT0); // Configure light properties

    // ---------------- SKYBOX SETUP ----------------
    // Creates a large textured cube surrounding the scene so the player appears to be inside an environment.
    sky->boxInit();
    foodSystem->init(sky); // Initialize sky
    foodSystem->spawnFoods(10); // Spawn food

    float skyBottomY = sky->pos.y - sky->scale.y * 0.5f;
    float floorY = skyBottomY + 3.0f;

    float halfX = sky->scale.x * 0.5f;
    float halfZ = sky->scale.z * 0.5f;
     // Load textures for each face of the cube
    sky->myTex[0].loadTexture("images/side2.png");
    sky->myTex[1].loadTexture("images/side_hole.png");
    sky->myTex[2].loadTexture("images/floor.png");
    sky->myTex[3].loadTexture("images/floor.png");
    sky->myTex[4].loadTexture("images/side2.png");
    sky->myTex[5].loadTexture("images/side1.png");

    // ---------- INITIALIZE PLAYER MODEL ----------
    // Load the rat OBJ model that the player controls.
    player = new _player();
    player->init("models/weretiger/tris.md2");
    player->physics.pos = {0, floorY, 0};

    // --- SETTING WHERE THE HOLE IS ---
    // Bottom-left corner of the back face
    mouseHolePos.x = -halfX + 30.0f;
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

    float distance = cam->distance;; // how far camera stays from player

    float radYaw = glm::radians(cam->yaw);
    float radPitch = glm::radians(cam->pitch);

    // Orbit around player
    cam->eye.x = cam->des.x + distance * cos(radPitch) * sin(radYaw);
    cam->eye.y = cam->des.y + distance * sin(radPitch);
    cam->eye.z = cam->des.z + distance * cos(radPitch) * cos(radYaw);

    // --- CAMERA WALL COLLISION ---
    float halfX = sky->scale.x * 0.48f;
    float halfZ = sky->scale.z * 0.48f;
    float maxDist = cam->distance;
    float safeDist = maxDist;

    // Check X walls
    if (cam->eye.x < -halfX) {
        float t = (-halfX - cam->des.x) / (cam->eye.x - cam->des.x);
        safeDist = maxDist * t;
    }
    if (cam->eye.x > halfX) {
        float t = (halfX - cam->des.x) / (cam->eye.x - cam->des.x);
        safeDist = maxDist * t;
    }

    // Check Z walls
    if (cam->eye.z < -halfZ) {
        float t = (-halfZ - cam->des.z) / (cam->eye.z - cam->des.z);
        safeDist = maxDist * t;
    }
    if (cam->eye.z > halfZ) {
        float t = (halfZ - cam->des.z) / (cam->eye.z - cam->des.z);
        safeDist = maxDist * t;
    }

    // Recalculate eye using SAFE distance
    cam->eye.x = cam->des.x + safeDist * cos(radPitch) * sin(radYaw);
    cam->eye.y = cam->des.y + safeDist * sin(radPitch);
    cam->eye.z = cam->des.z + safeDist * cos(radPitch) * cos(radYaw);

    cam->setUpCamera();  // Set camera position and orientation

    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);

    sky->drawBox();

    glPopAttrib();

    float floorY = (sky->pos.y - sky->scale.y * 0.5f) + 1;

    FrameRenderData frame;

    // -------- PLAYER --------
    frame.player.pos = player->physics.pos;
    frame.player.rotY = player->rot.y;
    frame.player.model = player->model;

    // -------- FOOD --------
    std::vector<FoodRenderData> foodRenderList;

    for (auto& f : foodSystem->foods) {
        FoodRenderData fd;
        fd.pos = f->physics.pos;
        fd.rotY = f->rot.y;
        fd.model = f->model;

        foodRenderList.push_back(fd);
    }

    frame.foods = &foodRenderList;

    // -------- FARTS --------
    std::vector<FartRenderData> fartRenderList;

    for (auto& f : abilities->farts) {
        FartRenderData fd;
        fd.pos = f.pos;
        fartRenderList.push_back(fd);
    }

    frame.farts = &fartRenderList;

    // -------- MOUSE HOLE --------
    frame.mouseHolePos = mouseHolePos;
    frame.mouseHoleRadius = mouseHoleRadius;

    // ---------- WORLD RENDER STATE ----------
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    // -------- RENDER --------
    renderer->renderFrame(frame);

    // ---------- UI RENDER SETUP ----------
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1920, 0, 1080);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT);

    // Disable 3D stuff
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // ---------- DRAW UI ----------
    ui->draw(
        1920, 1080,
        score,
        abilities->dashCooldownTimer,
        abilities->dashCooldown,
        abilities->canDash,
        abilities->fartTimer,
        abilities->fartCooldown,
        abilities->canFart
    );

    // ---------- RESTORE STATE ----------
    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void _Scene::updatePlayer(float dt) {
    float speed = 35.0f * dt; // Player movement speed scaled by frame time

    glm::vec3 moveVec(0.0f);

    // Camera forward (from eye --> target)
    glm::vec3 forwardRaw = glm::vec3(
        cam->des.x - cam->eye.x,
        0.0f,
        cam->des.z - cam->eye.z
    );

    glm::vec3 camForward = glm::length(forwardRaw) > 0.0001f
        ? glm::normalize(forwardRaw)
        : glm::vec3(0,0,-1);


    // Camera right (perpendicular)
    glm::vec3 rightRaw = glm::cross(camForward, glm::vec3(0,1,0));

    glm::vec3 camRight = glm::length(rightRaw) > 0.0001f
        ? glm::normalize(rightRaw)
        : glm::vec3(1,0,0);


    // WASD relative to camera
    if (GetAsyncKeyState('W') & 0x8000) moveVec += camForward;
    if (GetAsyncKeyState('S') & 0x8000) moveVec -= camForward;
    if (GetAsyncKeyState('A') & 0x8000) moveVec -= camRight;
    if (GetAsyncKeyState('D') & 0x8000) moveVec += camRight;

    // Save movement direction for pushing
    if (!abilities->isDashing()) {
        player->physics.pos += moveVec * speed;
    }

    // Rotate the rat so it faces the direction it moves
    if (glm::length(moveVec) > 0.0f) {
        moveVec = glm::normalize(moveVec);
        playerMoveDir = moveVec;

        player->rot.y = glm::degrees(atan2(moveVec.x, moveVec.z)) - 90.0f; // Rotate rat with movement
        player->model->actionTrigger = player->model->RUN;
    } else {
        player->model->actionTrigger = player->model->STAND;
    }
    player->model->actions();

    // Sky box boundary: Prevents player from leaving the skybox
    float halfX = sky->scale.x * 0.47f;
    float halfZ = sky->scale.z * 0.47f;

    if (player->physics.pos.x < -halfX) player->physics.pos.x = -halfX;
    if (player->physics.pos.x > halfX)  player->physics.pos.x = halfX;
    if (player->physics.pos.z < -halfZ) player->physics.pos.z = -halfZ;
    if (player->physics.pos.z > halfZ)  player->physics.pos.z = halfZ;
}

void _Scene::updateScene(float dt) {
    float floorY = (sky->pos.y - sky->scale.y * 0.5f) + 1;

    foodSystem->update(dt, floorY);
    foodSystem->handleCollisions();
    foodSystem->handlePlayerCollisions(player, playerMoveDir);
    foodSystem->checkFoodInHole(mouseHolePos, mouseHoleRadius, score, dt);

    abilities->handleInput(playerMoveDir, player->physics.pos);
    abilities->update(dt, player->physics.pos, playerMoveDir);

    updatePlayer(dt);
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
