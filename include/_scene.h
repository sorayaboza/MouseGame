#ifndef _SCENE_H
#define _SCENE_H

#include <vector>
#include <_common.h>
#include <_foodsystem.h>
#include <_lighting.h>
#include <_inputs.h>
#include <_texloader.h>
#include <_skybox.h>
#include <_camera.h>
#include <_food.h>
#include <_player.h>
#include <_ui.h>
#include <_abilities.h>
#include <_renderer.h>
#include <_enemy.h>
#include <_sounds.h>
#include <_leveltheme.h>

enum GameState {
    GS_LANDING,
    GS_PLAYING,
    GS_PAUSED,
    GS_GAME_OVER,
    GS_LEVEL_COMPLETE,
    GS_WIN
};

class _Scene
{
public:
    _Scene();
    virtual ~_Scene();

    GLint initGL();
    void  reSize(GLint, GLint);
    void  drawScene();
    void  updateScene(float dt);
    void  WndProc(HWND, UINT, WPARAM, LPARAM);

    void handleCollisions();
    void handlePlayerCollisions();
    void updatePlayer(float dt);
    void checkFoodInHole(float dt);

    _lighting*    myLight;
    _inputs*      keyMS;
    _texLoader*   myTex;
    _skyBox*      sky;
    _camera*      cam;
    _player*      player;
    _food*        heldFood = nullptr;
    _foodsystem*  foodSystem;
    _ui*          ui;
    _abilities*   abilities;
    _renderer*    renderer;
    _sounds*      sounds;
    _leveltheme* leveltheme;

    // ── Enemies: one cat + N dogs (varies per level) ────────────
    _enemy*               cat;
    std::vector<_enemy*>  dogs;

    glm::vec3 mouseHolePos;
    float     mouseHoleRadius;

    GameState gameState;
    int       currentLevel;
    int       score;
    int       foodThisLevel;
    int       dogsThisLevel;
    float     stateTimer;

    static const int FOOD_PER_LEVEL[3];
    static const int DOGS_PER_LEVEL[3];

    void startLevel(int level);
    void clearEnemies();
    void drawLandingScreen();
    void drawGameOverScreen();
    void drawLevelCompleteScreen();
    void drawWinScreen();
    void drawPauseScreen();
};

#endif
