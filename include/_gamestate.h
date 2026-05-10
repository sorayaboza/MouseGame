#ifndef _GAMESTATE_H
#define _GAMESTATE_H

class _Scene;   // forward declaration
class _texLoader;

class _gamestate
{
public:
    _gamestate();
    ~_gamestate();

    void draw(_Scene* scene);

    int slotHovered(_Scene* s);
    int pauseHoveredButton(_Scene* s);

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

private:
    void drawLandingScreen(_Scene* s);
    void drawLandingInfo(_Scene* s);
    void drawCredits(_Scene* s);
    void drawGameOverScreen(_Scene* s);
    void drawLevelCompleteScreen(_Scene* s);
    void drawWinScreen(_Scene* s);
    void drawPauseScreen(_Scene* s);
    void drawHelpInGame(_Scene* s);
    void drawBossIntro(_Scene* s);
    void drawMegaHitBanner(_Scene* s);
    void drawTexturedBackdrop(_Scene* s, bool useTitle = false);
    // Draw a full-screen background using an arbitrary texture
    // (used for pause / game over / win screens which have their
    // own hand-drawn artwork).
    void drawCustomBackdrop(_texLoader* tex, float darkenAlpha = 0.18f);
    void drawSlotMenu(_Scene* s, const char* title);
};

#endif
