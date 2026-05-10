/*
PURPOSE:
Main game controller for the entire project.

This file handles:
- Game states and level flow
- Player updates and rendering
- Enemy spawning, AI updates, and rendering
- Camera setup
- Input handling
- Collision checks
- Food spawning and collection
- Ability updates (dash/fart)
- Audio and sound effects
- HUD/UI rendering
- Save/load systems
- Skybox and environment rendering
- Level transitions and reset logic

_scene.cpp acts as the central manager that connects all gameplay systems together.
*/
#include "_scene.h"
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <windows.h>
#include <glm/gtc/matrix_transform.hpp>

float slideFriction  = 0.99f;
float scale          = 2.0f;
glm::vec3 playerMoveDir;

// Food per level
// Per-level food counts (kept moderate per user request)
const int _Scene::FOOD_PER_LEVEL[3] = { 5, 10, 14 };
// Number of dog enemies in addition to the main cat
const int _Scene::DOGS_PER_LEVEL[3] = { 0, 1, 2 };

// ================================================================
_Scene::_Scene()
{
    myLight       = new _lighting();
    keyMS         = new _inputs();
    myTex         = new _texLoader();
    menuBg        = new _texLoader();
    menuBgWithTitle = new _texLoader();
    pauseBg       = new _texLoader();
    gameOverBg    = new _texLoader();
    winBg         = new _texLoader();
    megaHitTex    = new _texLoader();
    sky           = new _skyBox();
    cam           = new _camera();
    foodSystem    = new _foodsystem();
    ui            = new _ui();
    abilities     = new _abilities();
    renderer      = new _renderer();
    sounds        = new _sounds();
    cat           = new _enemy();
    leveltheme    = new _leveltheme();
    terrainSystem = new _terrainsystem();
    gamestate     = new _gamestate();
    // dogs vector starts empty and is filled per-level

    mouseHoleRadius = 6.0f;
    gameState    = GS_LANDING;
    currentLevel = 1;
    score        = 0;
    stateTimer   = 0.0f;
    bossIntroTimer = 0.0f;
    gameOverMusicDelay = 0.0f;
    megaHitTimer = 0.0f;
    mouseX       = 0;
    mouseY       = 0;
    landingHover = -1;
    landingInfoPage = 0;
    prevHoverIdx = -1;
    prevGameState = GS_LANDING;
    foodThisLevel= FOOD_PER_LEVEL[0];
}
_Scene::~_Scene() { delete gamestate; }

// ================================================================
//  initGL
// ================================================================
GLint _Scene::initGL()
{
    glewInit();
    glClearColor(0.05f, 0.05f, 0.08f, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_NORMALIZE);

    // Warm kitchen ambient
    GLfloat gAmb[] = {0.42f, 0.38f, 0.32f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gAmb);

    myLight->setLight(GL_LIGHT0);

    // Secondary cool fill light (soft blue from above)
    glEnable(GL_LIGHT1);
    GLfloat lp1[] = {  0.0f, 80.0f,  0.0f, 1.0f};
    GLfloat ld1[] = { 0.85f, 0.78f, 0.65f, 1.0f};
    GLfloat la1[] = { 0.10f, 0.09f, 0.07f, 1.0f};
    GLfloat ls1[] = { 0.6f,  0.6f,  0.5f,  1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, lp1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  ld1);
    glLightfv(GL_LIGHT1, GL_AMBIENT,  la1);
    glLightfv(GL_LIGHT1, GL_SPECULAR, ls1);

    // ── Player (mouse) ────────────────────────────────────────────
    float skyBottomY = sky->pos.y - sky->scale.y * 0.5f;
    float floorY     = skyBottomY + 3.0f;
    float halfX      = sky->scale.x * 0.5f;
    float halfZ      = sky->scale.z * 0.5f;

    player = new _player();
    player->physics.pos = {0, floorY, 0};

    sky = new _skyBox();
    sky->boxInit();

    // ── Mouse hole on back wall (matches painted hole in side_hole.png) ──
    // The painted hole appears at (u=0.80, v_bottom=0.01) in the texture.
    // The back wall texture is HORIZONTALLY MIRRORED (u=1 at world x=-half,
    // u=0 at world x=+half), so u=0.80 maps to world x ≈ -66.
    // Empirically the hole pixel cluster centroid is at world (-66, -107).
    // We sit the magnetic disc just in front of the wall.
    mouseHolePos.x = -halfX * 0.60f;          // ≈ -66 with halfX=110
    mouseHolePos.y = floorY;
    mouseHolePos.z = -halfZ + 6.0f;           // 6 units in front of back wall

    foodSystem->init(sky);
    // Pre-spawn level 1 contents so the room is populated when the
    // player presses Enter; but stay on the landing screen first.
    startLevel(1);
    gameState = GS_LANDING;

    // ── Start background music (loops automatically) ─────────────
    sounds->iniSounds();
    sounds->playMusic((char*)"sounds/pause_menu.mp3");   // we boot to landing

    // ── Load Dishonored-style menu backdrop texture ──────────────
    menuBg->loadTexture("images/menu_bg.png");
    menuBgWithTitle->loadTexture("images/menu_bg_w_title.png");
    pauseBg->loadTexture("images/pause.png");
    gameOverBg->loadTexture("images/you_lose.png");
    winBg->loadTexture("images/you_win.png");
    megaHitTex->loadTexture("images/hit_text1.png");

    return true;
}

// ================================================================
//  startLevel
// ================================================================
void _Scene::clearEnemies() {
    for (auto* d : dogs) delete d;
    dogs.clear();
}

void _Scene::startLevel(int level) {
    currentLevel  = level;
    foodThisLevel = FOOD_PER_LEVEL[level - 1];
    dogsThisLevel = DOGS_PER_LEVEL[level - 1];

    // Wipe any active fart clouds, dash state, and cooldowns from
    // the previous level so they don't leak across.
    if (abilities) abilities->clearAll();

    // Boss-level intro: 3 seconds of "REVENGE OF THE FOOD" overlay
    bossIntroTimer = (level == 3) ? 3.0f : 0.0f;

    // ── Per-level room size ────────────────────────────────────
    // Level 3 has more enemies + more food, so give it a roomier arena.
    // IMPORTANT: this MUST happen before computing halfX/halfZ – the
    // old order computed dimensions then changed scale, which left
    // every spawn position from level 2's room.
    if (level == 3) {
        sky->scale = { 300.0f, 220.0f, 300.0f };  // ~36% larger floor
    } else {
        sky->scale = { 220.0f, 220.0f, 220.0f };  // levels 1 and 2
    }

    // Compute room half-extents AFTER the scale change.
    float halfX  = sky->scale.x * 0.5f;
    float halfZ  = sky->scale.z * 0.5f;
    float floorY = (sky->pos.y - sky->scale.y * 0.5f) + 3.0f;

    terrainSystem->clear();
    terrainSystem->spawn(level, halfX, halfZ, floorY, player->physics.pos);

    for (auto* f : foodSystem->foods) delete f;
    foodSystem->foods.clear();
    foodSystem->spawnFoods(foodThisLevel, level);

    // Spawn dogs FIRST (no init yet)
    clearEnemies();
    for (int i = 0; i < dogsThisLevel; i++) {
        _enemy* d = new _enemy();
        dogs.push_back(d);
    }

    leveltheme->apply(level, sky, player, cat, dogs);

    // ── Position everyone for a fresh level ────────────────────

    // Player – front-LEFT side of the room (positive Z, negative X)
    player->physics.pos = { -halfX * 0.55f, floorY, halfZ * 0.70f };
    player->physics.velocity = { 0, 0, 0 };

    // Cat – BACK-RIGHT corner (negative Z, positive X)
    cat->pos = { halfX * 0.75f, floorY, -halfZ * 0.65f };
    cat->state = _enemy::PATROL;
    cat->facingAngle = 3.14159f;
    cat->facing = { sinf(cat->facingAngle), 0, cosf(cat->facingAngle) };
    cat->chaseJustStarted = false;

    // Dogs – spread along the BACK of the room
    for (size_t i = 0; i < dogs.size(); i++) {
        float t = (dogs.size() == 1) ? 0.0f
                : ((float)i / (dogs.size() - 1) - 0.5f);
        dogs[i]->pos = {
            t * halfX * 1.0f,
            floorY,
            -halfZ * 0.65f
        };
        dogs[i]->state = _enemy::PATROL;
        dogs[i]->facingAngle = 0.0f;
        dogs[i]->facing = { 0, 0, 1 };
        dogs[i]->chaseJustStarted = false;
    }

    // ── Recompute mouse-hole position for this level ──
    // The painted hole on the back-wall texture sits in slightly
    // different UV positions per level.  Because the wall is mirrored
    // when applied, world X is mapped from a horizontally-flipped UV.
    //   Level 1 (kitchen)    – UV 0.20  → world x = -halfX * 0.60
    //   Level 2 (neon brick) – UV 0.20  → world x = -halfX * 0.60
    //   Level 3 (beach)      – UV 0.22  → world x = -halfX * 0.56
    //                                       (sand-castle door is closer
    //                                        to the centre)
    float holeXFactor = (level == 3) ? 0.60f : 0.60f;
    mouseHolePos.x = -halfX * holeXFactor;
    mouseHolePos.y = floorY;
    mouseHolePos.z = -halfZ + 6.0f;

    stateTimer = 0.0f;
    gameState  = GS_PLAYING;
}

// ================================================================
//  reSize
// ================================================================
void _Scene::reSize(GLint width, GLint height)
{
    GLfloat aspect = (GLfloat)width / (GLfloat)height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(60, aspect, 0.1, 2000);
    glMatrixMode(GL_MODELVIEW);  glLoadIdentity();
}

// ================================================================
//  Build SaveData from current state
// ================================================================
void _Scene::buildSaveData(SaveData& out)
{
    out.level     = currentLevel;
    out.score     = score;
    out.playerPos = player->physics.pos;

    out.foods.clear();
    for (auto* f : foodSystem->foods) {
        SavedFood sf;
        sf.foodType    = f->foodType;
        sf.displaySize = f->displaySize;
        sf.pos         = f->physics.pos;
        sf.velocity    = f->physics.velocity;
        sf.texturePath = f->texturePath;
        out.foods.push_back(sf);
    }

    out.cat.pos         = cat->pos;
    out.cat.facingAngle = cat->facingAngle;

    out.dogs.clear();
    for (auto* d : dogs) {
        SavedEnemy se;
        se.pos         = d->pos;
        se.facingAngle = d->facingAngle;
        out.dogs.push_back(se);
    }
}

// ================================================================
//  Apply loaded data: regenerate level then overwrite positions
// ================================================================
void _Scene::applyLoadedData(const SaveData& in)
{
    // First do a normal level start so all the assets/textures load
    // and the right number of dogs are spawned.
    startLevel(in.level);

    // Override score (startLevel doesn't touch it but we want to be sure)
    score = in.score;

    // Override player position
    player->physics.pos = in.playerPos;

    // Replace food list
    for (auto* f : foodSystem->foods) delete f;
    foodSystem->foods.clear();

    for (const auto& sf : in.foods) {
        int t = sf.foodType;
        _food* nf = new _food(
            sf.texturePath,
            sf.displaySize,
            sf.foodType
        );
        nf->physics.pos      = sf.pos;
        nf->physics.velocity = sf.velocity;
        foodSystem->foods.push_back(nf);
    }

    // Override enemy positions
    cat->pos         = in.cat.pos;
    cat->facingAngle = in.cat.facingAngle;
    cat->facing.x    = sinf(cat->facingAngle);
    cat->facing.z    = cosf(cat->facingAngle);

    // Match dog count to saved data (in case of mismatch, take min)
    size_t nD = std::min(dogs.size(), in.dogs.size());
    for (size_t i = 0; i < nD; i++) {
        dogs[i]->pos         = in.dogs[i].pos;
        dogs[i]->facingAngle = in.dogs[i].facingAngle;
        dogs[i]->facing.x    = sinf(dogs[i]->facingAngle);
        dogs[i]->facing.z    = cosf(dogs[i]->facingAngle);
    }

    gameState = GS_PLAYING;
}

// ================================================================
//  drawScene
// ================================================================
void _Scene::drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // ── Camera follow (orbit) ────────────────────────────────────
    cam->des.x = player->physics.pos.x;
    cam->des.y = player->physics.pos.y;
    cam->des.z = player->physics.pos.z;

    float dist     = cam->distance;
    float radYaw   = glm::radians(cam->yaw);
    float radPitch = glm::radians(cam->pitch);
    cam->eye.x = cam->des.x + dist * cosf(radPitch) * sinf(radYaw);
    cam->eye.y = cam->des.y + dist * sinf(radPitch);
    cam->eye.z = cam->des.z + dist * cosf(radPitch) * cosf(radYaw);

    // Wall-clip camera
    float hX = sky->scale.x * 0.48f;
    float hZ = sky->scale.z * 0.48f;
    float safeDist = dist;
    if (cam->eye.x < -hX) { float t = (-hX - cam->des.x) / (cam->eye.x - cam->des.x);
                            if (t > 0 && t < 1) safeDist = std::min(safeDist, dist*t); }
    if (cam->eye.x >  hX) { float t = ( hX - cam->des.x) / (cam->eye.x - cam->des.x);
                            if (t > 0 && t < 1) safeDist = std::min(safeDist, dist*t); }
    if (cam->eye.z < -hZ) { float t = (-hZ - cam->des.z) / (cam->eye.z - cam->des.z);
                            if (t > 0 && t < 1) safeDist = std::min(safeDist, dist*t); }
    if (cam->eye.z >  hZ) { float t = ( hZ - cam->des.z) / (cam->eye.z - cam->des.z);
                            if (t > 0 && t < 1) safeDist = std::min(safeDist, dist*t); }
    safeDist *= 0.95f;
    cam->eye.x = cam->des.x + safeDist * cosf(radPitch) * sinf(radYaw);
    cam->eye.y = cam->des.y + safeDist * sinf(radPitch);
    cam->eye.z = cam->des.z + safeDist * cosf(radPitch) * cosf(radYaw);

    cam->setUpCamera();

    terrainSystem->draw(); // TERRAIN

    // ── Skybox ───────────────────────────────────────────────────
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    sky->drawBox();
    glPopAttrib();

    // ── Enemy LOS cones (cat + dogs) ─────────────────────────────
    cat->drawLOS();
    for (auto* d : dogs) d->drawLOS();

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    // ── Build render frame data ──────────────────────────────────
    FrameRenderData frame;
    frame.player.pos   = player->physics.pos;
    frame.player.rotY  = player->rot.y;
    frame.player.model = player->model;

    std::vector<FoodRenderData> foodList;
    for (auto& f : foodSystem->foods) {
        FoodRenderData fd;
        fd.pos     = f->physics.pos;
        fd.size    = f->displaySize;
        fd.texture = f->texture;
        foodList.push_back(fd);
    }
    frame.foods = &foodList;

    std::vector<FartRenderData> fartList;
    for (auto& f : abilities->farts) {
        FartRenderData fd;
        fd.pos = f.pos; fd.life = f.life;
        fartList.push_back(fd);
    }
    frame.farts = &fartList;

    frame.mouseHolePos    = mouseHolePos;
    frame.mouseHoleRadius = mouseHoleRadius;
    frame.cameraPos       = glm::vec3(cam->eye.x, cam->eye.y, cam->eye.z);

    renderer->renderFrame(frame);



    // Draw enemies (cat + dogs)
    cat->draw();
    for (auto* d : dogs) d->draw();

    // ── HUD overlay ──────────────────────────────────────────────
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, 1920, 0, 1080);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);   // CRITICAL: stop the 3-D scene's texture
                                // from modulating our 2-D HUD colours.
                                // Without this, the black backing rectangles
                                // get tinted by whatever texture was last
                                // bound (skybox, terrain, etc.) which made
                                // the UI look faded.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    ui->draw(1920, 1080, score,
             abilities->dashCooldownTimer, abilities->dashCooldown, abilities->canDash,
             abilities->fartTimer,         abilities->fartCooldown, abilities->canFart);

    // ── Top-right: Level + Food remaining (black panel, gold text) ──
    char lbuf[64];
    sprintf(lbuf, "Level %d / 3   Food left: %d",
            currentLevel, (int)foodSystem->foods.size());

    // Solid black backing
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.92f);
    glBegin(GL_QUADS);
        glVertex2f(1590, 1020); glVertex2f(1910, 1020);
        glVertex2f(1910, 1065); glVertex2f(1590, 1065);
    glEnd();

    // Bright gold border
    glColor4f(1.0f, 0.82f, 0.20f, 1.0f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(1590, 1020); glVertex2f(1910, 1020);
        glVertex2f(1910, 1065); glVertex2f(1590, 1065);
    glEnd();
    glLineWidth(1.0f);

    // Bright gold text
    glColor3f(1.0f, 0.82f, 0.20f);
    glRasterPos2f(1605, 1037);
    for (const char* c = lbuf; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    // Centre warning: any enemy state
    bool anyChasing = (cat->state == _enemy::CHASE);
    bool anyDistracted = (cat->state == _enemy::DISTRACTED);
    for (auto* d : dogs) {
        if (d->state == _enemy::CHASE) anyChasing = true;
        if (d->state == _enemy::DISTRACTED) anyDistracted = true;
    }
    if (anyChasing) {
        float p = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.008f);
        glColor4f(1.0f, 0.18f, 0.18f, p);
        glRasterPos2f(800, 1040);
        const char* warn = "!!! AN ENEMY IS CHASING YOU !!!";
        for (const char* c = warn; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    } else if (anyDistracted) {
        glColor3f(0.95f, 0.85f, 0.10f);
        glRasterPos2f(820, 1040);
        const char* msg = "Enemy distracted!";
        for (const char* c = msg; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glPopAttrib();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();

    gamestate->draw(this);
}

// ================================================================
//  updatePlayer
// ================================================================
void _Scene::updatePlayer(float dt)
{
    float speed = 70.0f * dt;
    glm::vec3 moveVec(0.0f);

    glm::vec3 forwardRaw(cam->des.x - cam->eye.x, 0.0f, cam->des.z - cam->eye.z);
    glm::vec3 camForward = glm::length(forwardRaw) > 0.0001f
        ? glm::normalize(forwardRaw) : glm::vec3(0,0,-1);
    glm::vec3 rightRaw  = glm::cross(camForward, glm::vec3(0,1,0));
    glm::vec3 camRight  = glm::length(rightRaw) > 0.0001f
        ? glm::normalize(rightRaw) : glm::vec3(1,0,0);

    if (GetAsyncKeyState('W')      & 0x8000) moveVec += camForward;
    if (GetAsyncKeyState('S')      & 0x8000) moveVec -= camForward;
    if (GetAsyncKeyState('A')      & 0x8000) moveVec -= camRight;
    if (GetAsyncKeyState('D')      & 0x8000) moveVec += camRight;

    if (!abilities->isDashing())
        player->physics.pos += moveVec * speed;

    // Track whether player is moving (drives the procedural-mouse bob animation)
    if (glm::length(moveVec) > 0.0f) {
        moveVec = glm::normalize(moveVec);
        playerMoveDir = moveVec;
        // Same convention as enemies: rotation about Y is
        // (degrees(facingAngle) - 90) so the MD2 +X axis ends up
        // pointing along (sin(facingAngle), 0, cos(facingAngle)).
        float facingAngle = atan2f(moveVec.x, moveVec.z);
        player->rot.y = glm::degrees(facingAngle) - 90.0f;
        player->isMoving = true;
        player->animTime += dt;
    } else {
        player->isMoving = false;
    }

    // ── Animation priority: ATTACK (fart) > JUMP (dash) > RUN > STAND ──
    if (abilities->isFartAnimating()) {
        player->model->actionTrigger = player->model->ATTACK;
    } else if (abilities->isDashing()) {
        player->model->actionTrigger = player->model->JUMP;
    } else if (player->isMoving) {
        player->model->actionTrigger = player->model->RUN;
    } else {
        player->model->actionTrigger = player->model->STAND;
    }
    player->model->actions();

    // ── Arrow keys rotate the camera ─────────────────────────────
    //   ←/→  yaw    (orbit camera left/right around the player)
    //   ↑/↓  pitch  (tilt camera up/down)
    // Convention matches the right-mouse drag: arrow LEFT spins the
    // camera right around the player (so the world appears to spin left).
    float rotSpeed = 90.0f * dt;   // 90°/sec
    if (GetAsyncKeyState(VK_LEFT)  & 0x8000) cam->yaw   += rotSpeed;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) cam->yaw   -= rotSpeed;
    if (GetAsyncKeyState(VK_UP)    & 0x8000) cam->pitch += rotSpeed * 0.6f;
    if (GetAsyncKeyState(VK_DOWN)  & 0x8000) cam->pitch -= rotSpeed * 0.6f;
    if (cam->pitch > 80.0f) cam->pitch = 80.0f;
    if (cam->pitch <  5.0f) cam->pitch =  5.0f;


    float hX = sky->scale.x * 0.47f;
    float hZ = sky->scale.z * 0.47f;
    if (player->physics.pos.x < -hX) player->physics.pos.x = -hX;
    if (player->physics.pos.x >  hX) player->physics.pos.x =  hX;
    if (player->physics.pos.z < -hZ) player->physics.pos.z = -hZ;
    if (player->physics.pos.z >  hZ) player->physics.pos.z =  hZ;
}

// ================================================================
//  updateScene
// ================================================================
void _Scene::updateScene(float dt) {
    float halfX = sky->scale.x * 0.5f;
    float halfZ = sky->scale.z * 0.5f;
    // ─────────────────────────────────────────────────────────────
    //  GLOBAL SOUND DISPATCH
    // ─────────────────────────────────────────────────────────────
    //  MUSIC SELECTION
    //
    //  Main menu / pause / save / load / help → pause_menu.mp3
    //  Gameplay levels 1 & 2 (and end screens) → bg_music.mp3
    //  Gameplay level 3                       → menu_music.mp3
    //
    //  All three are looped via _sounds::playMusic() which won't
    //  restart the same file twice.  When paused, the pause loop
    //  replaces the gameplay music entirely (no overlap).
    // ─────────────────────────────────────────────────────────────
    auto isMenuOrPauseState = [](GameState s) {
        return s == GS_LANDING || s == GS_LANDING_INFO ||
               s == GS_LANDING_CREDITS || s == GS_LOAD_MENU ||
               s == GS_PAUSED || s == GS_SAVE_MENU ||
               s == GS_HELP_INGAME;
    };

    if (gameState == GS_GAME_OVER) {
        // Caught: let game_over.mp3 SFX play alone for ~3 seconds, then
        // start the pause/menu loop underneath the screen.
        if (gameOverMusicDelay > 0.0f) {
            gameOverMusicDelay -= dt;
            sounds->stopMusic();
        } else {
            sounds->playMusic((char*)"sounds/pause_menu.mp3");
        }
    }
    else if (gameState == GS_LEVEL_COMPLETE) {
        // Use the menu/pause loop on the Level Complete screen.
        sounds->playMusic((char*)"sounds/pause_menu.mp3");
    }
    else if (isMenuOrPauseState(gameState)) {
        sounds->playMusic((char*)"sounds/pause_menu.mp3");
    } else if (gameState == GS_PLAYING && currentLevel == 1) {
        sounds->playMusic((char*)"sounds/level1.mp3");
    } else if (gameState == GS_PLAYING && currentLevel == 2) {
        sounds->playMusic((char*)"sounds/level2.mp3");
    } else if (gameState == GS_PLAYING && currentLevel == 3) {
        sounds->playMusic((char*)"sounds/level3.mp3");
    } else {
        // Fallback (e.g. WIN screen, end states without their own music)
        sounds->playMusic((char*)"sounds/bg_music.mp3");
    }

    // ── State-transition SFX ──
    if (gameState != prevGameState) {
        // (Pause-open sting removed – the pause music itself signals
        //  the transition now.)

        // Caught – game over
        if (gameState == GS_GAME_OVER) {
            sounds->playSounds((char*)"sounds/game_over.mp3");
            gameOverMusicDelay = 3.0f;   // ~length of the SFX
        }

        // Level cleared
        if (gameState == GS_LEVEL_COMPLETE)
            sounds->playSounds((char*)"sounds/level_complete.mp3");

        // Beat the whole game (level 3 + win)
        if (gameState == GS_WIN)
            sounds->playSounds((char*)"sounds/game_complete_level3.mp3");

        prevGameState = gameState;
    }

    // ── Hover SFX REMOVED – was causing distortion when sweeping
    //     across buttons.  Click sounds remain.
    //     Still tracking prevHoverIdx in case we want to re-enable.
    if (gameState == GS_LANDING && landingHover != prevHoverIdx) {
        prevHoverIdx = landingHover;
    } else if (gameState != GS_LANDING) {
        prevHoverIdx = -1;
    }

    // ── Player ability SFX (one-shot edge triggers) ──
    if (abilities) {
        if (abilities->dashJustTriggered)
            sounds->playSounds((char*)"sounds/dash.mp3");
        if (abilities->fartJustTriggered)
            sounds->playSounds((char*)"sounds/fart.mp3");
        abilities->consumeSfxFlags();
    }

    // ── Enemy chase-start SFX ──
    if (cat && cat->chaseJustStarted) {
        sounds->playSounds((char*)"sounds/cat_chasing.mp3");
        cat->consumeSfxFlag();
    }
    for (auto* d : dogs) {
        if (d->chaseJustStarted) {
            sounds->playSounds((char*)"sounds/dog_chasing.mp3");
            d->consumeSfxFlag();
        }
    }

    // ── Enemy footsteps while chasing (real loop, not retriggered SFX) ──
    // Using playLoop/stopLoop so the sound STOPS when chase ends instead
    // of ringing out the remaining queued one-shots.
    bool anyChasing = (cat && cat->state == _enemy::CHASE);
    for (auto* d : dogs)
        if (d->state == _enemy::CHASE) anyChasing = true;
    if (anyChasing && gameState == GS_PLAYING)
        sounds->playLoop("sounds/enemy_running.mp3");
    else
        sounds->stopLoop();

    // ── Food-collected SFX (one ding per food this frame) ──
    if (foodSystem && foodSystem->foodCollectedThisFrame > 0) {
        sounds->playSounds((char*)"sounds/food_collected.mp3");
        foodSystem->foodCollectedThisFrame = 0;
    }

    // ── MEGA-HIT: dash into food triggers banner + SFX + bonus points ──
    if (foodSystem && foodSystem->megaHitsThisFrame > 0) {
        sounds->playSounds((char*)"sounds/bang.mp3");
        // Bonus 5 points per food hit while dashing
        score += 5 * foodSystem->megaHitsThisFrame;
        // Reset the banner timer (1.5s display)
        megaHitTimer = 1.5f;
        foodSystem->megaHitsThisFrame = 0;
    }
    if (megaHitTimer > 0.0f) megaHitTimer -= dt;

    if (gameState != GS_PLAYING) return;

    // ── Decrement the boss-intro overlay timer ──
    if (bossIntroTimer > 0.0f) bossIntroTimer -= dt;

    float floorY = (sky->pos.y - sky->scale.y * 0.5f) + 3.0f;
    float hX     = sky->scale.x * 0.47f;
    float hZ     = sky->scale.z * 0.47f;

    cat->pos.y = floorY;
    for (auto* d : dogs) d->pos.y = floorY;
    player->physics.pos.y = floorY;

    foodSystem->update(dt, floorY);
    foodSystem->handleCollisions();
    foodSystem->handlePlayerCollisions(player, playerMoveDir,
                                        abilities && abilities->isDashing());
    foodSystem->checkFoodInHole(mouseHolePos, mouseHoleRadius, score, dt);

    abilities->handleInput(playerMoveDir, player->physics.pos);
    abilities->update(dt, player->physics.pos, playerMoveDir);

    updatePlayer(dt);

    // Update cat (the main enemy)
    bool caught = cat->update(dt, player->physics.pos,
                               abilities->farts, hX, hZ);
    // Update each dog; first catch wins
    for (auto* d : dogs) {
        if (caught) break;
        caught = d->update(dt, player->physics.pos,
                            abilities->farts, hX, hZ);
    }
    if (caught) {
        gameState = GS_GAME_OVER;
        return;
    }

    if (foodSystem->foods.empty()) {
        gameState = (currentLevel >= 3) ? GS_WIN : GS_LEVEL_COMPLETE;
    }

    // TERRAIN STUFF
    terrainSystem->update(dt, halfX);
    terrainSystem->handleFoodCollision(foodSystem->foods);

    if (terrainSystem->checkPlayerCollision(player)) {
        gameState = GS_GAME_OVER;
    }
}

// ================================================================
//  Helper: try saving/loading a slot.  Returns true on success.
// ================================================================
static bool tryDoSave(_Scene* s, int slot) {
    SaveData sd;
    s->buildSaveData(sd);
    return _savegame::save(slot, sd);
}
static bool tryDoLoad(_Scene* s, int slot) {
    SaveData sd;
    if (!_savegame::load(slot, sd)) return false;
    s->applyLoadedData(sd);
    return true;
}

// ================================================================
//  WndProc
// ================================================================
void _Scene::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    // ── KEYBOARD ─────────────────────────────────────────────────
    case WM_KEYDOWN:
        // ESC behaviour by state
        if (wParam == VK_ESCAPE) {
            sounds->playSounds((char*)"sounds/button_click.mp3");
            switch (gameState) {
                case GS_PLAYING:        gameState = GS_PAUSED;     break;
                case GS_PAUSED:         gameState = GS_PLAYING;    break;
                case GS_HELP_INGAME:    gameState = GS_PLAYING;    break;
                case GS_SAVE_MENU:      gameState = GS_PAUSED;     break;
                case GS_LOAD_MENU:      gameState = GS_LANDING;    break;
                case GS_LANDING_INFO:    gameState = GS_LANDING;    break;
                case GS_LANDING_CREDITS: gameState = GS_LANDING;    break;
                case GS_GAME_OVER:       score = 0; gameState = GS_LANDING;  break;
                case GS_WIN:             score = 0; gameState = GS_LANDING;  break;
                case GS_LANDING:        PostQuitMessage(0);        break;
                default: break;
            }
            break;
        }

        // 'H' toggles help during gameplay/help screen
        if (wParam == 'H' || wParam == 'h') {
            if (gameState == GS_PLAYING)     gameState = GS_HELP_INGAME;
            else if (gameState == GS_HELP_INGAME) gameState = GS_PLAYING;
            break;
        }

        // ── PAUSE menu number-key shortcuts ──────────────────────
        if (gameState == GS_PAUSED) {
            if (wParam >= '1' && wParam <= '4')
                sounds->playSounds((char*)"sounds/button_click.mp3");
            switch (wParam) {
                case '1': gameState = GS_SAVE_MENU;    break;  // Save
                case '2': gameState = GS_PLAYING;      break;  // Continue
                case '3': gameState = GS_HELP_INGAME;  break;  // Controls
                case '4': // Exit to main menu
                    score = 0;
                    gameState = GS_LANDING;
                    break;
            }
            break;
        }

        // ── SAVE / LOAD slot selection by number key ─────────────
        if (gameState == GS_SAVE_MENU) {
            int slot = 0;
            if (wParam == '1') slot = 1;
            else if (wParam == '2') slot = 2;
            else if (wParam == '3') slot = 3;
            if (slot) {
                if (tryDoSave(this, slot))
                    sounds->playSounds((char*)"sounds/game_saved.mp3");
                gameState = GS_PAUSED;   // back to pause menu after saving
            }
            break;
        }
        if (gameState == GS_LOAD_MENU) {
            int slot = 0;
            if (wParam == '1') slot = 1;
            else if (wParam == '2') slot = 2;
            else if (wParam == '3') slot = 3;
            if (slot) {
                if (tryDoLoad(this, slot)) {
                    sounds->playSounds((char*)"sounds/game_loading.mp3");
                } else {
                    // empty slot → just bounce back to landing
                    gameState = GS_LANDING;
                }
            }
            break;
        }

        // ── LANDING menu number-key shortcuts (1-5) ───────────────
        if (gameState == GS_LANDING) {
            if (wParam >= '1' && wParam <= '5')
                sounds->playSounds((char*)"sounds/button_click.mp3");
            switch (wParam) {
                case '1': score = 0; startLevel(1);        break;  // Start
                case '2': gameState = GS_LOAD_MENU;        break;  // Load
                case '3': gameState = GS_LANDING_INFO;     break;  // Controls
                case '4': gameState = GS_LANDING_CREDITS;  break;  // Credits
                case '5': PostQuitMessage(0);              break;  // Exit
            }
            // Don't break – ENTER below also still triggers Start
        }

        // ── ENTER on landing/end screens ─────────────────────────
        if (wParam == VK_RETURN) {
            if (gameState == GS_LANDING) {
                score = 0; startLevel(1);
            }
            else if (gameState == GS_LEVEL_COMPLETE)
                startLevel(currentLevel + 1);
            else if (gameState == GS_GAME_OVER) {
                // "Try again" – fresh playthrough from level 1
                score = 0;
                startLevel(1);
            }
            else if (gameState == GS_WIN) {
                // After winning: back to main menu so they can pick again
                score = 0;
                gameState = GS_LANDING;
            }
            break;
        }
        break;

    // ── MOUSE BUTTONS (clicking menus + camera drag in play) ────
    case WM_LBUTTONDOWN: {
        // Convert window mouse to ortho coords (1920x1080 space)
        // We don't get window size here; use the values stored by WM_MOUSEMOVE
        if (gameState == GS_LANDING) {
            // landingHover is updated each frame in drawLandingScreen
            if (landingHover >= 0)
                sounds->playSounds((char*)"sounds/button_click.mp3");
            switch (landingHover) {
                case 0: score = 0; startLevel(1);        break;  // Start
                case 1: gameState = GS_LOAD_MENU;        break;  // Load
                case 2: gameState = GS_LANDING_INFO;     break;  // Controls
                case 3: gameState = GS_LANDING_CREDITS;  break;  // Credits
                case 4: PostQuitMessage(0);              break;  // Exit
            }
        }
        else if (gameState == GS_PAUSED) {
            int hov = gamestate->pauseHoveredButton(this);
            if (hov >= 0) sounds->playSounds((char*)"sounds/button_click.mp3");
            if (hov == 0) gameState = GS_SAVE_MENU;
            else if (hov == 1) gameState = GS_PLAYING;
            else if (hov == 2) gameState = GS_HELP_INGAME;
            else if (hov == 3) { score = 0; gameState = GS_LANDING; }
        }
        else if (gameState == GS_SAVE_MENU) {
            int slot = gamestate->slotHovered(this);
            if (slot >= 1 && slot <= 3) {
                sounds->playSounds((char*)"sounds/button_click.mp3");
                if (tryDoSave(this, slot))
                    sounds->playSounds((char*)"sounds/game_saved.mp3");
                gameState = GS_PAUSED;
            }
        }
        else if (gameState == GS_LOAD_MENU) {
            int slot = gamestate->slotHovered(this);
            if (slot >= 1 && slot <= 3) {
                sounds->playSounds((char*)"sounds/button_click.mp3");
                if (tryDoLoad(this, slot)) {
                    sounds->playSounds((char*)"sounds/game_loading.mp3");
                } else {
                    gameState = GS_LANDING;
                }
            }
        }
        break;
    }

    case WM_RBUTTONDOWN:
        if (gameState == GS_PLAYING) {
            cam->isRightMouseDown = true;
            ShowCursor(FALSE);
            RECT rect; GetClientRect(hWnd, &rect);
            POINT ul={rect.left,rect.top}, lr={rect.right,rect.bottom};
            ClientToScreen(hWnd,&ul); ClientToScreen(hWnd,&lr);
            rect.left=ul.x; rect.top=ul.y; rect.right=lr.x; rect.bottom=lr.y;
            ClipCursor(&rect);
            int cx=(rect.left+rect.right)/2, cy=(rect.top+rect.bottom)/2;
            SetCursorPos(cx,cy);
            cam->lastMouseX=cx; cam->lastMouseY=cy;
        }
        break;
    case WM_RBUTTONUP:
        cam->isRightMouseDown = false;
        ShowCursor(TRUE);
        ClipCursor(NULL);
        break;

    case WM_MOUSEMOVE: {
        // Track mouse for menu hover (convert client coords to ortho)
        // Game uses gluOrtho2D(0, 1920, 0, 1080) – Y up.
        int wx = LOWORD(lParam);
        int wy = HIWORD(lParam);
        RECT rect; GetClientRect(hWnd, &rect);
        int cw = rect.right - rect.left;
        int ch = rect.bottom - rect.top;
        if (cw <= 0) cw = 1;
        if (ch <= 0) ch = 1;
        mouseX = (int)(wx * 1920.0f / cw);
        mouseY = (int)((ch - wy) * 1080.0f / ch);

        if (cam->isRightMouseDown && gameState == GS_PLAYING) {
            POINT p; GetCursorPos(&p);
            int dx = p.x - cam->lastMouseX;
            int dy = p.y - cam->lastMouseY;
            cam->yaw   -= dx * 0.25f;
            cam->pitch += dy * 0.25f;
            if (cam->pitch > 80.0f) cam->pitch = 80.0f;
            if (cam->pitch <  5.0f) cam->pitch =  5.0f;
            SetCursorPos(cam->lastMouseX, cam->lastMouseY);
        }
        break;
    }
    }
}
