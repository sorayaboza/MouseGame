// ================================================================
//  _scene.cpp  |  Mouse Heist  –  Tom & Jerry kitchen game
// ================================================================
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
const int _Scene::FOOD_PER_LEVEL[3] = { 8, 12, 16 };
// Number of dog enemies in addition to the main cat
const int _Scene::DOGS_PER_LEVEL[3] = { 0, 1, 2 };

// ================================================================
_Scene::_Scene()
{
    myLight    = new _lighting();
    keyMS      = new _inputs();
    myTex      = new _texLoader();
    sky        = new _skyBox();
    cam        = new _camera();
    foodSystem = new _foodsystem();
    ui         = new _ui();
    abilities  = new _abilities();
    renderer   = new _renderer();
    sounds     = new _sounds();
    cat        = new _enemy();
    leveltheme = new _leveltheme();
    // dogs vector starts empty and is filled per-level

    mouseHoleRadius = 6.0f;
    gameState    = GS_LANDING;
    currentLevel = 1;
    score        = 0;
    stateTimer   = 0.0f;
    foodThisLevel= FOOD_PER_LEVEL[0];
}
_Scene::~_Scene() {}

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
    sounds->playMusic((char*)"sounds/bg_music.mp3");

    return true;
}

// ================================================================
//  startLevel
// ================================================================
void _Scene::clearEnemies()
{
    for (auto* d : dogs) delete d;
    dogs.clear();
}

void _Scene::startLevel(int level)
{
    currentLevel  = level;
    foodThisLevel = FOOD_PER_LEVEL[level - 1];
    dogsThisLevel = DOGS_PER_LEVEL[level - 1];

    for (auto* f : foodSystem->foods) delete f;
    foodSystem->foods.clear();
    foodSystem->spawnFoods(foodThisLevel);

    // Spawn dogs FIRST (no init yet)
    clearEnemies();
    for (int i = 0; i < dogsThisLevel; i++) {
        _enemy* d = new _enemy();
        dogs.push_back(d);
    }

    leveltheme->apply(level, sky, player, cat, dogs);

    // NOW position enemies AFTER init
    float halfX = sky->scale.x * 0.5f;
    float halfZ = sky->scale.z * 0.5f;

    // Cat position
    cat->pos = { halfX - 30.0f, 0.0f, halfZ - 30.0f };

    // Dog positions
    for (int i = 0; i < dogs.size(); i++) {
        float angle = (i * 2.0f + 1.0f) * 3.14159f / (dogs.size() + 1);
        dogs[i]->pos.x = cosf(angle) * (halfX * 0.55f);
        dogs[i]->pos.y = 0.0f;
        dogs[i]->pos.z = sinf(angle) * (halfZ * 0.55f);
    }

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
//  2-D helpers
// ================================================================
static void begin2D(int w, int h)
{
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST); glDisable(GL_TEXTURE_2D);
}
static void end2D()
{
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
    glEnable(GL_LIGHTING); glEnable(GL_DEPTH_TEST); glEnable(GL_TEXTURE_2D);
}
static void drawTextAt(float x, float y, const char* s,
                        void* font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (const char* c = s; *c; c++) glutBitmapCharacter(font, *c);
}
static void drawStroke(float x, float y, float sc, const char* s)
{
    glPushMatrix();
    glTranslatef(x, y, 0); glScalef(sc, sc, 1);
    for (const char* c = s; *c; c++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();
}
static void fillRect(float x, float y, float w, float h,
                      float r, float g, float b, float a = 1)
{
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();
}

// ================================================================
//  LANDING SCREEN  –  with full HOW TO PLAY section
// ================================================================
void _Scene::drawLandingScreen()
{
    begin2D(1920, 1080);
    float cx = 960, cy = 540;

    // Dark gradient background
    glBegin(GL_QUADS);
    glColor3f(0.04f, 0.04f, 0.06f); glVertex2f(0, 0);    glVertex2f(1920, 0);
    glColor3f(0.10f, 0.06f, 0.04f); glVertex2f(1920, 1080); glVertex2f(0, 1080);
    glEnd();

    // ── Title banner ─────────────────────────────────────────────
    fillRect(cx-460, cy+330, 920, 130, 0.10f, 0.05f, 0.02f, 0.92f);
    glColor3f(0.85f, 0.65f, 0.10f);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx-460, cy+330); glVertex2f(cx+460, cy+330);
    glVertex2f(cx+460, cy+460); glVertex2f(cx-460, cy+460);
    glEnd();
    glLineWidth(1.0f);

    glLineWidth(3.5f); glColor3f(0.95f, 0.72f, 0.12f);
    drawStroke(cx-380, cy+380, 0.55f, "MOUSE HEIST");
    glLineWidth(1.0f);
    glColor3f(0.75f, 0.62f, 0.42f);
    drawTextAt(cx-185, cy+340, "A food collection mouse game",
               GLUT_BITMAP_HELVETICA_18);

    // ── HOW TO PLAY panel ────────────────────────────────────────
    fillRect(cx-450, cy-280, 920, 580, 0.07f, 0.05f, 0.03f, 0.92f);
    glColor3f(0.65f, 0.50f, 0.10f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx-450, cy-280); glVertex2f(cx+450, cy-280);
    glVertex2f(cx+450, cy+300); glVertex2f(cx-450, cy+300);
    glEnd();
    glLineWidth(1.0f);

    // Section title
    glColor3f(0.95f, 0.78f, 0.18f);
    drawTextAt(cx-110, cy+255, "HOW TO PLAY",
               GLUT_BITMAP_TIMES_ROMAN_24);
    // Divider
    glColor3f(0.55f, 0.42f, 0.08f);
    glBegin(GL_LINES);
    glVertex2f(cx-380, cy+240); glVertex2f(cx+380, cy+240);
    glEnd();

    // ── CONTROLS column (left) ───────────────────────────────────
    glColor3f(0.95f, 0.82f, 0.30f);
    drawTextAt(cx-410, cy+200, "CONTROLS", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.85f, 0.78f, 0.62f);
    struct Row { const char* k; const char* d; };
    Row controls[] = {
        { "WASD",         "Move the rat"             },
        { "Arrow Keys",   "Rotate camera (yaw/pitch)" },
        { "Right Mouse",  "Drag to rotate camera"    },
        { "SPACE",        "Dash forward"             },
        { "F",            "Fart (distract human)"    },
        { "ESC",          "Pause / Resume"           },
        { "ENTER",        "Start / Continue"         }
    };
    for (int i = 0; i < 7; i++) {
        glColor3f(0.95f, 0.82f, 0.30f);
        drawTextAt(cx-410, cy+165 - i*28, controls[i].k,
                   GLUT_BITMAP_HELVETICA_18);
        glColor3f(0.85f, 0.78f, 0.62f);
        drawTextAt(cx-265, cy+165 - i*28, controls[i].d,
                   GLUT_BITMAP_HELVETICA_18);
    }

    // ── GOAL & SCORING column (right) ────────────────────────────
    glColor3f(0.95f, 0.82f, 0.30f);
    drawTextAt(cx+30, cy+200, "GOAL & SCORING", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.85f, 0.78f, 0.62f);
    const char* goal[] = {
        "Push food into your mouse hole",
        "Each food collected   =  +10 points",
        "Collect EVERY food to clear the level",
        "Avoid the cats' & dogs' RED vision cones!",
        "Touching any enemy  =  GAME OVER"
    };
    for (int i = 0; i < 5; i++)
        drawTextAt(cx+30, cy+165 - i*28, goal[i], GLUT_BITMAP_HELVETICA_18);

    // ── LEVELS section ───────────────────────────────────────────
    glColor3f(0.55f, 0.42f, 0.08f);
    glBegin(GL_LINES);
    glVertex2f(cx-380, cy+5); glVertex2f(cx+380, cy+5);
    glEnd();

    glColor3f(0.95f, 0.82f, 0.30f);
    drawTextAt(cx-50, cy-25, "LEVELS", GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.85f, 0.78f, 0.62f);
    char lbuf[160];
    sprintf(lbuf, "Level 1  -  %d foods  -  1 cat only  -  +%d points",
            FOOD_PER_LEVEL[0], FOOD_PER_LEVEL[0]*10);
    drawTextAt(cx-410, cy-60, lbuf, GLUT_BITMAP_HELVETICA_18);
    sprintf(lbuf, "Level 2  -  %d foods  -  1 cat + 1 dog (faster)  -  +%d points",
            FOOD_PER_LEVEL[1], FOOD_PER_LEVEL[1]*10);
    drawTextAt(cx-410, cy-90, lbuf, GLUT_BITMAP_HELVETICA_18);
    sprintf(lbuf, "Level 3  -  %d foods  -  1 cat + 2 dogs (fastest)  -  +%d points",
            FOOD_PER_LEVEL[2], FOOD_PER_LEVEL[2]*10);
    drawTextAt(cx-410, cy-120, lbuf, GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.55f, 0.85f, 0.55f);
    int maxScore = (FOOD_PER_LEVEL[0] + FOOD_PER_LEVEL[1] + FOOD_PER_LEVEL[2]) * 10;
    sprintf(lbuf, "MAX SCORE = %d points (clear all 3 levels)", maxScore);
    drawTextAt(cx-200, cy-160, lbuf, GLUT_BITMAP_HELVETICA_18);

    // ── PRO TIPS ─────────────────────────────────────────────────
    glColor3f(0.55f, 0.42f, 0.08f);
    glBegin(GL_LINES);
    glVertex2f(cx-380, cy-185); glVertex2f(cx+380, cy-185);
    glEnd();
    glColor3f(0.95f, 0.82f, 0.30f);
    drawTextAt(cx-55, cy-215, "PRO TIPS", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.85f, 0.78f, 0.62f);
    drawTextAt(cx-410, cy-245,
        "GREEN cone = patrol, YELLOW = distracted, RED = chasing.",
        GLUT_BITMAP_HELVETICA_18);
    drawTextAt(cx-410, cy-270,
        "Use FART to escape when the human spots you, then DASH to safety!",
        GLUT_BITMAP_HELVETICA_18);

    // ── Pulsing prompt ───────────────────────────────────────────
    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1.0f, 0.85f, 0.30f, 0.45f + 0.55f * pulse);
    drawTextAt(cx-180, cy-320, "Press  ENTER  to Begin",
               GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.40f, 0.36f, 0.30f);
    drawTextAt(cx-100, 28, "3 Levels   |   Tom & Jerry style",
               GLUT_BITMAP_HELVETICA_12);

    end2D();
}

// ================================================================
//  GAME OVER SCREEN
// ================================================================
void _Scene::drawGameOverScreen()
{
    begin2D(1920, 1080);
    float cx=960, cy=540;
    fillRect(0,0,1920,1080, 0,0,0, 0.82f);
    fillRect(cx-380, cy-90, 760, 280, 0.18f, 0.03f, 0.03f, 0.96f);
    glColor3f(0.85f,0.10f,0.10f); glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx-380,cy-90);glVertex2f(cx+380,cy-90);glVertex2f(cx+380,cy+190);glVertex2f(cx-380,cy+190);
    glEnd(); glLineWidth(1.0f);

    glLineWidth(3.5f); glColor3f(1.0f, 0.18f, 0.18f);
    drawStroke(cx-310, cy+115, 0.42f, "CAUGHT!");
    glLineWidth(1.0f);

    char buf[64];
    sprintf(buf, "Final Score: %d points", score);
    glColor3f(1.0f, 0.85f, 0.85f);
    drawTextAt(cx-130, cy+30, buf, GLUT_BITMAP_TIMES_ROMAN_24);

    sprintf(buf, "You reached Level %d / 3", currentLevel);
    glColor3f(0.85f, 0.65f, 0.65f);
    drawTextAt(cx-130, cy-10, buf, GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.85f, 0.65f, 0.65f);
    drawTextAt(cx-220, cy-50,
               "An enemy caught the rat! Try again?",
               GLUT_BITMAP_HELVETICA_18);

    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1.0f, 0.92f, 0.50f, 0.40f + 0.60f * pulse);
    drawTextAt(cx-200, cy-130,
               "Press ENTER to play again   |   ESC to quit",
               GLUT_BITMAP_HELVETICA_18);

    end2D();
}

// ================================================================
//  LEVEL COMPLETE SCREEN
// ================================================================
void _Scene::drawLevelCompleteScreen()
{
    begin2D(1920, 1080);
    float cx=960, cy=540;
    fillRect(0,0,1920,1080, 0,0,0, 0.78f);

    fillRect(cx-420, cy-80, 840, 280, 0.03f, 0.14f, 0.04f, 0.96f);
    glColor3f(0.20f, 0.85f, 0.30f); glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx-420,cy-80);glVertex2f(cx+420,cy-80);
    glVertex2f(cx+420,cy+200);glVertex2f(cx-420,cy+200);
    glEnd(); glLineWidth(1.0f);

    char title[64];
    sprintf(title, "LEVEL %d COMPLETE!", currentLevel);
    glLineWidth(3.5f); glColor3f(0.30f, 1.0f, 0.40f);
    drawStroke(cx-360, cy+125, 0.36f, title);
    glLineWidth(1.0f);

    char sbuf[64];
    sprintf(sbuf, "Score: %d points", score);
    glColor3f(0.85f, 1.0f, 0.85f);
    drawTextAt(cx-100, cy+40, sbuf, GLUT_BITMAP_TIMES_ROMAN_24);

    if (currentLevel < 3) {
        char nbuf[128];
        sprintf(nbuf, "Next: Level %d  -  %d foods  -  %d dog%s + the cat!",
                currentLevel+1, FOOD_PER_LEVEL[currentLevel],
                DOGS_PER_LEVEL[currentLevel],
                DOGS_PER_LEVEL[currentLevel] == 1 ? "" : "s");
        glColor3f(0.75f, 0.92f, 0.78f);
        drawTextAt(cx-260, cy-5, nbuf, GLUT_BITMAP_HELVETICA_18);

        char pbuf[64];
        sprintf(pbuf, "Possible bonus: +%d points", FOOD_PER_LEVEL[currentLevel] * 10);
        glColor3f(0.55f, 0.82f, 0.58f);
        drawTextAt(cx-130, cy-40, pbuf, GLUT_BITMAP_HELVETICA_18);
    }

    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1, 0.95f, 0.50f, 0.40f + 0.60f * pulse);
    drawTextAt(cx-160, cy-110, "Press ENTER to continue",
               GLUT_BITMAP_TIMES_ROMAN_24);

    end2D();
}

// ================================================================
//  WIN SCREEN
// ================================================================
void _Scene::drawWinScreen()
{
    begin2D(1920, 1080);
    float cx=960, cy=540;
    fillRect(0,0,1920,1080, 0.02f, 0.02f, 0.08f, 1.0f);

    fillRect(cx-450, cy-50, 900, 290, 0.05f, 0.05f, 0.20f, 0.97f);
    glColor3f(0.85f, 0.68f, 0.12f); glLineWidth(2.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx-450,cy-50);glVertex2f(cx+450,cy-50);
    glVertex2f(cx+450,cy+240);glVertex2f(cx-450,cy+240);
    glEnd(); glLineWidth(1.0f);

    glLineWidth(4.0f); glColor3f(0.95f, 0.80f, 0.10f);
    drawStroke(cx-415, cy+165, 0.42f, "VICTORY!");
    glLineWidth(1.0f);

    glColor3f(0.88f, 0.92f, 1.0f);
    drawTextAt(cx-340, cy+85,
        "You collected all the food across all 3 levels!",
        GLUT_BITMAP_HELVETICA_18);

    char sbuf[64];
    sprintf(sbuf, "Final Score: %d points", score);
    glColor3f(0.95f, 0.80f, 0.10f);
    drawTextAt(cx-130, cy+40, sbuf, GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(0.70f, 0.78f, 0.92f);
    drawTextAt(cx-280, cy+0,
        "The mouse lives to eat another day! Congratulations.",
        GLUT_BITMAP_HELVETICA_18);

    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1, 0.92f, 0.50f, 0.40f + 0.60f * pulse);
    drawTextAt(cx-180, cy-30, "Press ENTER to play again",
               GLUT_BITMAP_TIMES_ROMAN_24);

    end2D();
}

// ================================================================
//  PAUSE SCREEN
// ================================================================
void _Scene::drawPauseScreen()
{
    begin2D(1920, 1080);
    float cx=960, cy=540;
    fillRect(0, 0, 1920, 1080, 0, 0, 0, 0.55f);
    fillRect(cx-280, cy-80, 560, 240, 0.06f, 0.06f, 0.18f, 0.96f);
    glColor3f(0.85f, 0.68f, 0.12f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx-280,cy-80);glVertex2f(cx+280,cy-80);
    glVertex2f(cx+280,cy+160);glVertex2f(cx-280,cy+160);
    glEnd();

    glLineWidth(2.8f); glColor3f(1.0f, 0.92f, 0.32f);
    drawStroke(cx-180, cy+105, 0.40f, "PAUSED");
    glLineWidth(1.0f);

    glColor3f(0.85f, 0.85f, 0.85f);
    drawTextAt(cx-140, cy+40, "Press ESC to resume",   GLUT_BITMAP_HELVETICA_18);
    drawTextAt(cx-180, cy+5,  "Press ENTER to quit to menu", GLUT_BITMAP_HELVETICA_18);

    end2D();
}

// ================================================================
//  drawScene
// ================================================================
void _Scene::drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (gameState == GS_LANDING)        { drawLandingScreen();       return; }
    if (gameState == GS_GAME_OVER)      { drawGameOverScreen();      return; }
    if (gameState == GS_LEVEL_COMPLETE) { drawLevelCompleteScreen(); return; }
    if (gameState == GS_WIN)            { drawWinScreen();           return; }

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
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);

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

    if (gameState == GS_PAUSED) drawPauseScreen();
}

// ================================================================
//  updatePlayer
// ================================================================
void _Scene::updatePlayer(float dt)
{
    float speed = 50.0f * dt;
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
    if (GetAsyncKeyState(VK_LEFT)  & 0x8000) cam->yaw   -= rotSpeed;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) cam->yaw   += rotSpeed;
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
void _Scene::updateScene(float dt)
{
    if (gameState != GS_PLAYING) return;

    float floorY = (sky->pos.y - sky->scale.y * 0.5f) + 3.0f;
    float hX     = sky->scale.x * 0.47f;
    float hZ     = sky->scale.z * 0.47f;

    cat->pos.y = floorY;
    for (auto* d : dogs) d->pos.y = floorY;
    player->physics.pos.y = floorY;

    foodSystem->update(dt, floorY);
    foodSystem->handleCollisions();
    foodSystem->handlePlayerCollisions(player, playerMoveDir);
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
}

// ================================================================
//  WndProc
// ================================================================
void _Scene::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_RETURN:
            if (gameState == GS_LANDING)        { score = 0; startLevel(1); }
            else if (gameState == GS_LEVEL_COMPLETE) startLevel(currentLevel + 1);
            else if (gameState == GS_GAME_OVER || gameState == GS_WIN) {
                score = 0; startLevel(1); gameState = GS_LANDING;
            }
            else if (gameState == GS_PAUSED) {
                score = 0; gameState = GS_LANDING;
            }
            break;
        case VK_ESCAPE:
            if      (gameState == GS_PLAYING) gameState = GS_PAUSED;
            else if (gameState == GS_PAUSED)  gameState = GS_PLAYING;
            else if (gameState == GS_LANDING) PostQuitMessage(0);
            break;
        }
        break;

    case WM_RBUTTONDOWN:
        if (gameState == GS_PLAYING || gameState == GS_PAUSED) {
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
    case WM_MOUSEMOVE:
        if (cam->isRightMouseDown && gameState == GS_PLAYING) {
            POINT p; GetCursorPos(&p);
            int dx = p.x - cam->lastMouseX;
            int dy = p.y - cam->lastMouseY;
            // Drag right → world spins right (camera orbits left around player)
            // Drag down  → camera tilts down (look more horizontal)
            cam->yaw   -= dx * 0.25f;
            cam->pitch += dy * 0.25f;
            if (cam->pitch > 80.0f) cam->pitch = 80.0f;
            if (cam->pitch <  5.0f) cam->pitch =  5.0f;
            SetCursorPos(cam->lastMouseX, cam->lastMouseY);
        }
        break;
    }
}
