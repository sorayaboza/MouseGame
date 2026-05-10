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
#include <_savegame.h>
#include <_terrainsystem.h>
#include <_gamestate.h>

enum GameState {
    GS_LANDING,
    GS_LANDING_INFO,     // shows Controls/Goal/Levels info on top of landing
    GS_LANDING_CREDITS,  // shows credits page
    GS_PLAYING,
    GS_PAUSED,           // ESC pause menu (Save/Continue/Controls/Exit)
    GS_HELP_INGAME,      // 'H' help overlay during play, also Pause→Controls
    GS_SAVE_MENU,        // pick a slot to save into (Pause→Save)
    GS_LOAD_MENU,        // pick a slot to load from (Landing→Load)
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

    // ================= TERRAIN =================
    void drawTerrain();
    struct Terrain {
        int type; // 0 = bottle, 1 = crab
        glm::vec3 pos;
        glm::vec3 dir; // movement direction (for crabs)
        float moveTimer; // for crab direction switching
        _ModelLoaderMD2* model;
    };

    _lighting*      myLight;
    _inputs*        keyMS;
    _texLoader*     myTex;
    _texLoader*     menuBg;       // images/menu_bg.png – Dishonored-style backdrop
    _texLoader*     menuBgWithTitle; // images/menu_bg_w_title.png – landing-only art
    _texLoader*     pauseBg;         // images/pause.png – pause screen art
    _texLoader*     gameOverBg;      // images/you_lose.png – game over art (has title)
    _texLoader*     winBg;           // images/you_win.png – win screen art (has title)
    _skyBox*        sky;
    _camera*        cam;
    _player*        player;
    _food*          heldFood = nullptr;
    _foodsystem*    foodSystem;
    _ui*            ui;
    _abilities*     abilities;
    _renderer*      renderer;
    _sounds*        sounds;
    _leveltheme*    leveltheme;
    _terrainsystem* terrainSystem;
    _gamestate* gamestate;

    // ── Enemies: one cat + N dogs (varies per level) ────────────
    _enemy*               cat;
    std::vector<_enemy*>  dogs;

    glm::vec3 mouseHolePos;
    float     mouseHoleRadius;

    GameState gameState;
    GameState prevGameState;     // for SFX edge-detection on state changes
    int       currentLevel;
    int       score;
    int       foodThisLevel;
    int       dogsThisLevel;
    float     stateTimer;
    float     bossIntroTimer;   // counts down from 3.0 at start of level 3
    float     gameOverMusicDelay; // counts down before pause music starts on caught screen

    // ── MEGA-HIT visual ──
    // Counts down from 1.5s when the player dashes into food.
    // While > 0, drawScene() overlays the hit_text1.png banner.
    float       megaHitTimer;
    _texLoader* megaHitTex;       // images/hit_text1.png

    // Mouse cursor (window coords) – used by menu hover/click logic
    int       mouseX;
    int       mouseY;

    // ── Landing page menu ───────────────────────────────────────
    // 0 = Start Game, 1 = Load Game, 2 = Controls, 3 = Exit
    int       landingHover;       // updated each draw, used by click handler
    int       landingInfoPage;    // which info panel to show in GS_LANDING_INFO
    int       prevHoverIdx;       // for button-hover SFX edge detection

    static const int FOOD_PER_LEVEL[3];
    static const int DOGS_PER_LEVEL[3];

    void startLevel(int level);
    void clearEnemies();
    void drawLandingScreen();
    void drawGameOverScreen();
    void drawLevelCompleteScreen();
    void drawWinScreen();
    void drawPauseScreen();
    void drawHelpInGame();
    void drawBossIntro();           // boss-level overlay during start of level 3
    void drawMegaHitBanner();       // hit_text1.png banner when dashing into food
    void drawTexturedBackdrop();   // Dishonored-style PNG bg + grunge overlay
    void drawLandingInfo();    // info shown when "Controls" pressed on landing
    void drawCredits();        // credits page
    void drawSlotMenu(const char* title);   // shared by save+load menus

    // Save/load helpers
    void buildSaveData(SaveData& out);
    void applyLoadedData(const SaveData& in);
    int  pauseHoveredButton();              // 0..3 = save/cont/ctrl/exit
    int  slotHovered();                     // 1..3 if mouse over a slot, else 0
};

#endif
