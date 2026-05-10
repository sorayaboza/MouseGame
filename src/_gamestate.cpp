#include <windows.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include "_gamestate.h"
#include "_scene.h"

_gamestate::_gamestate() {}
_gamestate::~_gamestate() {}


// ================================================================
//  2-D helpers
// ================================================================
static void begin2D(int w, int h)
{
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST); glDisable(GL_TEXTURE_2D);

    // ── Anti-aliasing for sharper edges on lines, points ─────────
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

// Render stroke text with a sharp drop shadow (offset by 4px down-right).
// Caller sets line width before calling; the foreground colour must be
// set AFTER this call returns or use drawStrokeWithShadow which sets both.
static void drawStrokeShadow(float x, float y, float sc, const char* s,
                              float r, float g, float b,
                              float lineWidthShadow = 4.0f,
                              float lineWidthMain = 4.5f)
{
    // ── Drop shadow (dark, slightly offset) ─────────────────────
    glLineWidth(lineWidthShadow);
    glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
    glPushMatrix();
    glTranslatef(x + 4.0f, y - 4.0f, 0); glScalef(sc, sc, 1);
    for (const char* c = s; *c; c++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();

    // ── Main text in chosen colour ──────────────────────────────
    glLineWidth(lineWidthMain);
    glColor3f(r, g, b);
    glPushMatrix();
    glTranslatef(x, y, 0); glScalef(sc, sc, 1);
    for (const char* c = s; *c; c++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();

    glLineWidth(1.0f);
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
//  softTextBacking – soft, "blurred" dark rectangle to sit behind
//                    text so it stays readable over busy backgrounds.
//
//  Renders 4 stacked rectangles of decreasing size and increasing
//  opacity.  The outer rings are very faint and provide a soft
//  feathered edge; the inner core is a solid dark plate that
//  contrasts strongly with light/cream text.  Cheap and works
//  with raw OpenGL primitives (no actual blur shader).
// ================================================================
static void softTextBacking(float x, float y, float w, float h)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    struct Layer { float padX, padY, alpha; };
    Layer layers[5] = {
        { 22.0f, 16.0f, 0.10f },   // outermost faint glow
        { 16.0f, 12.0f, 0.22f },
        { 10.0f,  8.0f, 0.42f },
        {  5.0f,  4.0f, 0.62f },
        {  0.0f,  0.0f, 0.78f },   // solid core
    };

    // Deep purple that matches the menu artwork's palette.  Same
    // hue family as the torn-strip buttons, slightly darker so the
    // bright cream/yellow text on top still pops.
    for (int i = 0; i < 5; i++) {
        const Layer& L = layers[i];
        glColor4f(0.16f, 0.06f, 0.24f, L.alpha);
        glBegin(GL_QUADS);
        glVertex2f(x - L.padX,    y - L.padY);
        glVertex2f(x + w + L.padX, y - L.padY);
        glVertex2f(x + w + L.padX, y + h + L.padY);
        glVertex2f(x - L.padX,    y + h + L.padY);
        glEnd();
    }
}

// ================================================================
//  drawGrungeBackdrop – dark, atmospheric, deterministic
//
//  Uses a fixed PRNG seed so the texture doesn't shimmer between
//  frames.  Three layers:
//    1. Vertical gradient (deep blue-grey at top to near-black bottom)
//    2. Soft inky vignette (large dark blob in the corners)
//    3. Hairline scratches & specks scattered across the canvas
// ================================================================
static unsigned int g_grungeSeed = 12345u;
static unsigned int xorshift(unsigned int& s) {
    s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s;
}
static float frand(unsigned int& s) { return (xorshift(s) & 0xFFFFFF) / 16777216.0f; }

static void drawGrungeBackdrop()
{
    // ── Layer 1: vertical gradient ─────────────────────────────
    glBegin(GL_QUADS);
    // Top: deep cool grey
    glColor3f(0.10f, 0.10f, 0.13f); glVertex2f(0, 1080);    glVertex2f(1920, 1080);
    // Bottom: nearly black
    glColor3f(0.03f, 0.03f, 0.05f); glVertex2f(1920, 0);    glVertex2f(0, 0);
    glEnd();

    // ── Layer 2: corner vignette (soft dark blobs) ────────────
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    struct Blob { float cx, cy, r; };
    Blob blobs[6] = {
        {  120,  140, 600 },   // bottom-left
        { 1800,  100, 700 },   // bottom-right
        {  -50, 1100, 500 },   // top-left
        { 1950, 1000, 550 },   // top-right
        {  500,  500, 380 },   // mid-left
        { 1500,  600, 380 },   // mid-right
    };
    for (int b = 0; b < 6; b++) {
        glBegin(GL_TRIANGLE_FAN);
        glColor4f(0, 0, 0, 0.55f);
        glVertex2f(blobs[b].cx, blobs[b].cy);
        glColor4f(0, 0, 0, 0.0f);
        for (int i = 0; i <= 24; i++) {
            float a = i * 2.0f * 3.14159f / 24.0f;
            glVertex2f(blobs[b].cx + cosf(a) * blobs[b].r,
                       blobs[b].cy + sinf(a) * blobs[b].r);
        }
        glEnd();
    }

    // ── Layer 3: scratches & specks ─────────────────────────────
    unsigned int s = g_grungeSeed;
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < 90; i++) {
        float x = frand(s) * 1920.0f;
        float y = frand(s) * 1080.0f;
        float ang = frand(s) * 6.2832f;
        float len = 8.0f + frand(s) * 32.0f;
        float a   = 0.10f + frand(s) * 0.18f;
        glColor4f(0.85f, 0.78f, 0.62f, a);
        glVertex2f(x, y);
        glVertex2f(x + cosf(ang) * len, y + sinf(ang) * len);
    }
    glEnd();

    // Specks (tiny dots = single short lines)
    glBegin(GL_POINTS);
    for (int i = 0; i < 250; i++) {
        float x = frand(s) * 1920.0f;
        float y = frand(s) * 1080.0f;
        float a = 0.04f + frand(s) * 0.18f;
        glColor4f(0.80f, 0.74f, 0.55f, a);
        glVertex2f(x, y);
    }
    glEnd();

    // ── Layer 4: a few "ink splatter" patches for character ───
    s = g_grungeSeed + 999;
    for (int p = 0; p < 4; p++) {
        float bx = 200 + frand(s) * 1520.0f;
        float by = 100 + frand(s) * 880.0f;
        for (int j = 0; j < 14; j++) {
            float r = 4.0f + frand(s) * 22.0f;
            float dx = (frand(s) - 0.5f) * 90.0f;
            float dy = (frand(s) - 0.5f) * 90.0f;
            glBegin(GL_TRIANGLE_FAN);
            glColor4f(0, 0, 0, 0.18f);
            glVertex2f(bx + dx, by + dy);
            for (int k = 0; k <= 12; k++) {
                float a = k * 2.0f * 3.14159f / 12.0f;
                glVertex2f(bx + dx + cosf(a)*r, by + dy + sinf(a)*r);
            }
            glEnd();
        }
    }
}

// ================================================================
//  drawTornStrip – horizontal "torn paper" button as in the
//  Dishonored-style menu.  Hovered state lights up beige.
// ================================================================
static void drawTornStrip(float x, float y, float w, float h,
                          bool hovered)
{
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ── Drop shadow (offset down-right) ─────────────────────────
    if (!hovered) {
        glColor4f(0, 0, 0, 0.55f);
        glBegin(GL_QUADS);
        glVertex2f(x + 4, y - 4); glVertex2f(x + w + 4, y - 4);
        glVertex2f(x + w + 4, y + h - 4); glVertex2f(x + 4, y + h - 4);
        glEnd();
    }

    // ── Main strip body – COLOURFUL gradient that matches the
    //    Mouse Heist artwork (pinks, purples, warm food tones) ──
    if (hovered) {
        // Hover: warm orange/cheese gradient – pops against the
        // purple background and matches the food in the artwork.
        glBegin(GL_QUADS);
        glColor4f(1.00f, 0.78f, 0.30f, 0.97f); glVertex2f(x, y + h);
        glColor4f(1.00f, 0.55f, 0.25f, 0.97f); glVertex2f(x + w, y + h);
        glColor4f(0.95f, 0.40f, 0.25f, 0.97f); glVertex2f(x + w, y);
        glColor4f(0.90f, 0.35f, 0.30f, 0.97f); glVertex2f(x, y);
        glEnd();
    } else {
        // Default: deep purple/plum gradient – matches the artwork's
        // purple/violet background.  Saturated enough to read as a
        // distinct UI element, dark enough that white text stays
        // legible.  Diagonal gradient gives it a painterly feel.
        glBegin(GL_QUADS);
        glColor4f(0.42f, 0.18f, 0.55f, 0.94f); glVertex2f(x, y + h);     // top-left: violet
        glColor4f(0.55f, 0.20f, 0.50f, 0.94f); glVertex2f(x + w, y + h); // top-right: magenta
        glColor4f(0.32f, 0.12f, 0.40f, 0.94f); glVertex2f(x + w, y);     // bot-right: deep plum
        glColor4f(0.22f, 0.08f, 0.32f, 0.94f); glVertex2f(x, y);         // bot-left: dark purple
        glEnd();
    }

    // Sharp top + bottom edge lines for definition
    if (!hovered) {
        // Pink highlight on top, dark plum line on bottom
        glColor4f(0.95f, 0.55f, 0.85f, 0.80f);
        glBegin(GL_LINES);
        glVertex2f(x, y + h); glVertex2f(x + w, y + h);
        glEnd();
        glColor4f(0.10f, 0.04f, 0.18f, 0.95f);
        glBegin(GL_LINES);
        glVertex2f(x, y); glVertex2f(x + w, y);
        glEnd();
    } else {
        // Bright yellow rim on hover – matches the title's yellow letters
        glColor4f(1.0f, 0.92f, 0.30f, 0.95f);
        glBegin(GL_LINES);
        glVertex2f(x, y);     glVertex2f(x + w, y);
        glVertex2f(x, y + h); glVertex2f(x + w, y + h);
        glEnd();
    }

    // ── Ragged left edge – torn paper boundary (in dark plum) ─────
    unsigned int s = (unsigned int)(x * 3 + y * 7) | 1u;
    glColor4f(0.06f, 0.02f, 0.10f, 1.0f);
    glBegin(GL_TRIANGLE_STRIP);
    int steps = 24;
    for (int i = 0; i <= steps; i++) {
        float t   = (float)i / steps;
        float yy  = y + t * h;
        float jag = (frand(s) - 0.5f) * 18.0f;
        glVertex2f(x - 14, yy);
        glVertex2f(x + jag, yy);
    }
    glEnd();
}

// ================================================================
//  drawTexturedBackdrop  –  Dishonored-style PNG bg + procedural
//                          grunge overlay drawn on top.
//
//  This is the new "4K-feeling" backdrop:
//    1. Draw menu_bg.png filling the whole screen
//    2. Layer additional procedural grunge on top so the menu
//       feels alive and not just static wallpaper.
// ================================================================
void _gamestate::drawTexturedBackdrop(_Scene* s, bool useTitle)
{
    // ── Layer 1: textured background ─────────────────────────────
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    if (useTitle && s->menuBgWithTitle) s->menuBgWithTitle->BindTex();
    else if (s->menuBg)                 s->menuBg->BindTex();
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(0,    0);
        glTexCoord2f(1, 1); glVertex2f(1920, 0);
        glTexCoord2f(1, 0); glVertex2f(1920, 1080);
        glTexCoord2f(0, 0); glVertex2f(0,    1080);
    glEnd();
    glPopAttrib();

    // No procedural grunge overlay when using the title bg – the
    // hand-drawn artwork is already plenty textured.
    if (useTitle) {
        // BUT we still need to darken the busy artwork a bit so
        // text + buttons remain readable.  A subtle 22% black
        // overlay strengthens contrast without washing out the art.
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.22f);
        glBegin(GL_QUADS);
            glVertex2f(0, 0);    glVertex2f(1920, 0);
            glVertex2f(1920, 1080); glVertex2f(0, 1080);
        glEnd();
        return;
    }

    // ── Layer 2: procedural grunge overlay (non-title backdrops only) ─
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    unsigned int t = 8675309u;
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < 60; i++) {
        float x = frand(t) * 1920.0f;
        float y = frand(t) * 1080.0f;
        float ang = frand(t) * 6.2832f;
        float len = 8.0f + frand(t) * 24.0f;
        float a   = 0.06f + frand(t) * 0.10f;
        glColor4f(0.95f, 0.88f, 0.70f, a);
        glVertex2f(x, y);
        glVertex2f(x + cosf(ang) * len, y + sinf(ang) * len);
    }
    glEnd();
}

// ================================================================
//  drawCustomBackdrop – full-screen backdrop using an arbitrary
//                       texture (e.g., pause.png, you_lose.png,
//                       you_win.png).  Optional dark overlay on
//                       top to keep any prose text readable.
// ================================================================
void _gamestate::drawCustomBackdrop(_texLoader* tex, float darkenAlpha)
{
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    if (tex) tex->BindTex();
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(0,    0);
        glTexCoord2f(1, 1); glVertex2f(1920, 0);
        glTexCoord2f(1, 0); glVertex2f(1920, 1080);
        glTexCoord2f(0, 0); glVertex2f(0,    1080);
    glEnd();
    glPopAttrib();

    if (darkenAlpha > 0.001f) {
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, darkenAlpha);
        glBegin(GL_QUADS);
            glVertex2f(0, 0);    glVertex2f(1920, 0);
            glVertex2f(1920, 1080); glVertex2f(0, 1080);
        glEnd();
    }
}

// ================================================================
//  LANDING SCREEN  –  clean 4-button main menu
//
//  Buttons (top to bottom):
//      1. Start Game
//      2. Load Game
//      3. Controls
//      4. Exit (ESC)
//
//  Each is mouse-clickable and has a number-key shortcut (1-4).
//  ESC also triggers Exit.  All gameplay info is hidden behind
//  the "Controls" button, shown only when the user opens it.
// ================================================================
static const char* LANDING_LABELS[5] = {
    "1.  New Game",
    "2.  Load Game",
    "3.  Controls",
    "4.  Credits",
    "5.  Exit  (ESC)"
};

// Landing-page torn-strip geometry (1920×1080 ortho)
// Right-aligned buttons: ~640px wide, ending 90px from right edge.
static const float LMENU_X = 1190.0f;          // left edge of strip
static const float LMENU_W = 640.0f;           // strip width
static const float LMENU_H = 64.0f;            // strip height
static const float LMENU_GAP = 20.0f;          // vertical gap
static const float LMENU_TOP_BY = 720.0f;      // bottom-Y of TOP strip

// Returns 0..4 for hovered button, -1 for none
static int landingHoveredButtonAt(int mouseX, int mouseY)
{
    float mx = (float)mouseX;
    float my = (float)mouseY;
    if (mx < LMENU_X || mx > LMENU_X + LMENU_W) return -1;
    for (int i = 0; i < 5; i++) {
        float by = LMENU_TOP_BY - i * (LMENU_H + LMENU_GAP);
        if (my >= by && my <= by + LMENU_H) return i;
    }
    return -1;
}

void _gamestate::drawLandingScreen(_Scene* s)
{
    begin2D(1920, 1080);

    // ── Background art (title is baked into the artwork) ─────────
    drawTexturedBackdrop(s, true);   // use menu_bg_w_title.png

    // No stroke "MOUSE HEIST" – the title is part of the artwork.

    // ── 5 right-aligned torn-strip buttons ───────────────────────
    s->landingHover = landingHoveredButtonAt(mouseX, mouseY);
    for (int i = 0; i < 5; i++) {
        float by = LMENU_TOP_BY - i * (LMENU_H + LMENU_GAP);
        bool hovered = (i == s->landingHover);

        drawTornStrip(LMENU_X, by, LMENU_W, LMENU_H, hovered);

        // Right-aligned label.
        const char* lbl = LANDING_LABELS[i];
        int textPx = 0;
        for (const char* c = lbl; *c; c++)
            textPx += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        if (hovered) glColor3f(0.05f, 0.05f, 0.05f);    // dark on cream
        else         glColor3f(0.97f, 0.94f, 0.84f);    // bright cream on dark
        drawTextAt(LMENU_X + LMENU_W - textPx - 36, by + 24,
                   lbl, GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // ── Footer area – soft blurred backing + bright text for readability ─
    softTextBacking(60, 75, 640, 60);

    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1.0f, 0.92f, 0.55f, 0.85f + 0.15f * pulse);
    drawTextAt(75, 110,
               "Click a button or press the number key (1-5)",
               GLUT_BITMAP_HELVETICA_18);

    bool anySave = false;
    for (int sl = 1; sl <= _savegame::NUM_SLOTS; sl++)
        if (_savegame::peek(sl).exists) { anySave = true; break; }
    if (anySave) {
        glColor3f(0.65f, 1.0f, 0.65f);
        drawTextAt(75, 85, "Saved games available - click Load Game",
                   GLUT_BITMAP_HELVETICA_18);
    }

    end2D();
}


// ================================================================
//  LANDING INFO – Controls / Goal / Levels (the old How-To-Play
//  content, now hidden until user clicks the "Controls" button)
// ================================================================
void _gamestate::drawLandingInfo(_Scene* s)
{
    begin2D(1920, 1080);

    drawTexturedBackdrop(s);

    // ── Title (top, large stroke text) ──────────────────────────
    softTextBacking(340, 850, 760, 155);
    drawStrokeShadow(360, 920, 0.85f, "GAME CONTROLS", 0.95f, 0.92f, 0.78f);

    glColor3f(1.0f, 0.95f, 0.68f);
    drawTextAt(380, 870, "Everything you need to know",
               GLUT_BITMAP_TIMES_ROMAN_24);

    glColor4f(1.0f, 0.92f, 0.55f, 0.85f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(360, 855); glVertex2f(960, 855);
    glEnd();
    glLineWidth(1.0f);

    // ── Right-side info strips (one per row, horizontal layout) ─
    // Each row is a torn strip; key on left, description on right.
    const float SX = 1010.0f;     // strip left edge (right-aligned)
    const float SW = 820.0f;      // strip width
    const float SH = 50.0f;       // strip height
    const float SGAP = 10.0f;
    const float SY_TOP = 800.0f;  // bottom-Y of TOP strip

    struct Row { const char* k; const char* d; };
    Row controls[] = {
        { "WASD",         "Move the rat"             },
        { "Arrow Keys",   "Rotate camera (yaw/pitch)" },
        { "Right Mouse",  "Drag to rotate camera"    },
        { "SPACE",        "Dash forward"             },
        { "F",            "Fart - distract enemies"  },
        { "H",            "Show controls in-game"    },
        { "ESC",          "Pause menu"               }
    };
    const int N_CONTROLS = 7;
    for (int i = 0; i < N_CONTROLS; i++) {
        float by = SY_TOP - i * (SH + SGAP);
        drawTornStrip(SX, by, SW, SH, false);
        // Key (left, gold)
        glColor3f(1.0f, 0.85f, 0.30f);
        drawTextAt(SX + 24, by + 18, controls[i].k, GLUT_BITMAP_TIMES_ROMAN_24);
        // Description (right, cream)
        glColor3f(0.92f, 0.88f, 0.78f);
        drawTextAt(SX + 220, by + 18, controls[i].d, GLUT_BITMAP_HELVETICA_18);
    }

    // ── Goal section (left side of screen, with soft backing) ────
    softTextBacking(45, 540, 630, 260);  // GOAL header + 5 lines

    glLineWidth(3.0f); glColor3f(1.00f, 0.95f, 0.78f);
    drawStroke(80, 760, 0.36f, "GOAL");
    glLineWidth(1.0f);

    glColor4f(1.0f, 0.92f, 0.50f, 0.85f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(80, 740); glVertex2f(360, 740);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(1.0f, 0.98f, 0.92f);
    const char* goal[] = {
        "Push food into your mouse hole.",
        "Each food collected = +10 points.",
        "Collect EVERY food to clear the level.",
        "Avoid the cats' & dogs' RED vision cones!",
        "Touching any enemy = GAME OVER."
    };
    for (int i = 0; i < 5; i++)
        drawTextAt(80, 700 - i*32, goal[i], GLUT_BITMAP_HELVETICA_18);

    // ── Levels section (left side, below goal) ──────────────────
    softTextBacking(45, 325, 630, 220);  // LEVELS header + 4 lines

    glLineWidth(3.0f); glColor3f(1.00f, 0.95f, 0.78f);
    drawStroke(80, 500, 0.36f, "LEVELS");
    glLineWidth(1.0f);

    glColor4f(1.0f, 0.92f, 0.50f, 0.85f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(80, 480); glVertex2f(360, 480);
    glEnd();
    glLineWidth(1.0f);

    char lbuf[160];
    glColor3f(1.0f, 0.98f, 0.92f);
    sprintf(lbuf, "Level 1  -  %d foods  -  1 cat only",
            s->FOOD_PER_LEVEL[0]);
    drawTextAt(80, 440, lbuf, GLUT_BITMAP_HELVETICA_18);
    sprintf(lbuf, "Level 2  -  %d foods  -  1 cat + 1 dog",
            s->FOOD_PER_LEVEL[1]);
    drawTextAt(80, 408, lbuf, GLUT_BITMAP_HELVETICA_18);
    sprintf(lbuf, "Level 3  -  %d foods  -  1 cat + 2 dogs",
            s->FOOD_PER_LEVEL[2]);
    drawTextAt(80, 376, lbuf, GLUT_BITMAP_HELVETICA_18);

    glColor3f(0.65f, 1.0f, 0.65f);
    int maxScore = (s->FOOD_PER_LEVEL[0] + s->FOOD_PER_LEVEL[1] + s->FOOD_PER_LEVEL[2]) * 10;
    sprintf(lbuf, "Max possible score: %d", maxScore);
    drawTextAt(80, 340, lbuf, GLUT_BITMAP_HELVETICA_18);

    // ── Pro tips (left side, below levels) ──────────────────────
    softTextBacking(45, 180, 630, 155);  // PRO TIPS header + 2 lines

    glLineWidth(3.0f); glColor3f(1.00f, 0.95f, 0.78f);
    drawStroke(80, 290, 0.36f, "PRO TIPS");
    glLineWidth(1.0f);

    glColor4f(1.0f, 0.92f, 0.50f, 0.85f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(80, 270); glVertex2f(360, 270);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(1.0f, 0.98f, 0.92f);
    drawTextAt(80, 230,
        "GREEN cone = patrol, YELLOW = distracted, RED = chasing",
        GLUT_BITMAP_HELVETICA_18);
    drawTextAt(80, 198,
        "Use FART to escape when spotted, then DASH to safety!",
        GLUT_BITMAP_HELVETICA_18);

    // ── Bottom hint (with backing) ──────────────────────────────
    softTextBacking(60, 95, 350, 35);

    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1.0f, 0.92f, 0.55f, 0.85f + 0.15f * pulse);
    drawTextAt(80, 110, "Press ESC to go back",
               GLUT_BITMAP_HELVETICA_18);

    end2D();
}

// ================================================================
//  CREDITS PAGE
//
//  Two columns – one per credit – with each person's contributions
//  listed vertically (one bullet per line) under their name.
// ================================================================
void _gamestate::drawCredits(_Scene* s)
{
    begin2D(1920, 1080);

    // Background
    drawTexturedBackdrop(s);

    // ── Title (top, large stroke text) ──────────────────────────
    softTextBacking(340, 850, 760, 155);
    drawStrokeShadow(360, 920, 0.85f, "CREDITS", 0.95f, 0.92f, 0.78f);

    glColor3f(1.0f, 0.95f, 0.68f);
    drawTextAt(380, 870, "Mouse Heist - made by",
               GLUT_BITMAP_TIMES_ROMAN_24);

    glColor4f(1.0f, 0.92f, 0.55f, 0.85f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(360, 855); glVertex2f(960, 855);
    glEnd();
    glLineWidth(1.0f);

    // ── Two columns of contributors ─────────────────────────────
    struct Person { const char* name; const char* roles[6]; int n; };
    Person p1 = {
        "Soraya Boza",
        {
            "Art for images and models",
            "Programming",
            "Game design",
            "Testing",
            nullptr, nullptr
        },
        4
    };
    Person p2 = {
        "Harshil Taunk",
        {
            "Programming",
            "Testing",
            "Debugging",
            nullptr, nullptr, nullptr
        },
        3
    };

    // Two columns spread across the right portion of the screen
    float col1X = 380.0f;
    float col2X = 1050.0f;
    float nameY = 720.0f;
    float bulletStartY = 640.0f;
    float bulletStep = 42.0f;

    // Backing covers from below the lowest bullet up past the
    // title's top.  Title baseline=nameY=720 with scale 0.36 so
    // title top ≈ 754; lowest bullet at bulletStartY-(n-1)*step.
    {
        float bottom = bulletStartY - (p1.n - 1) * bulletStep - 18;
        float top    = nameY + 50;
        softTextBacking(col1X - 24, bottom, 430, top - bottom);
    }

    glLineWidth(3.5f); glColor3f(1.0f, 0.95f, 0.78f);
    drawStroke(col1X, nameY, 0.36f, p1.name);
    glLineWidth(1.0f);

    glColor4f(1.0f, 0.92f, 0.50f, 0.85f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(col1X, nameY - 18); glVertex2f(col1X + 380, nameY - 18);
    glEnd();
    glLineWidth(1.0f);

    for (int i = 0; i < p1.n; i++) {
        // Bright gold bullet
        glColor3f(1.0f, 0.92f, 0.40f);
        drawTextAt(col1X, bulletStartY - i * bulletStep, ">",
                   GLUT_BITMAP_TIMES_ROMAN_24);
        // Bright cream role text
        glColor3f(1.0f, 0.98f, 0.92f);
        drawTextAt(col1X + 30, bulletStartY - i * bulletStep, p1.roles[i],
                   GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // ── Column 2: Harshil ────────────────────────────────────────
    {
        float bottom = bulletStartY - (p2.n - 1) * bulletStep - 18;
        float top    = nameY + 50;
        softTextBacking(col2X - 24, bottom, 430, top - bottom);
    }

    glLineWidth(3.5f); glColor3f(1.0f, 0.95f, 0.78f);
    drawStroke(col2X, nameY, 0.36f, p2.name);
    glLineWidth(1.0f);

    glColor4f(1.0f, 0.92f, 0.50f, 0.85f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(col2X, nameY - 18); glVertex2f(col2X + 380, nameY - 18);
    glEnd();
    glLineWidth(1.0f);

    for (int i = 0; i < p2.n; i++) {
        glColor3f(1.0f, 0.92f, 0.40f);
        drawTextAt(col2X, bulletStartY - i * bulletStep, ">",
                   GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(1.0f, 0.98f, 0.92f);
        drawTextAt(col2X + 30, bulletStartY - i * bulletStep, p2.roles[i],
                   GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // ── Thanks line (centered, mid-screen, with backing) ───────
    softTextBacking(500, 260, 920, 80);
    drawStrokeShadow(540, 280, 0.42f, "Thanks for playing!", 1.0f, 0.95f, 0.78f);

    // ── Bottom hint ─────────────────────────────────────────────
    softTextBacking(60, 95, 350, 35);

    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1.0f, 0.92f, 0.55f, 0.85f + 0.15f * pulse);
    drawTextAt(80, 110, "Press ESC to go back",
               GLUT_BITMAP_HELVETICA_18);

    end2D();
}

// ================================================================
//  GAME OVER SCREEN
// ================================================================
void _gamestate::drawGameOverScreen(_Scene* s)
{
    begin2D(1920, 1080);

    // ── Game-over artwork (YOU LOSE title baked into the art) ────
    // Light dark overlay (10%) keeps the stats text readable
    // without dimming the illustration.
    drawCustomBackdrop(s->gameOverBg, 0.10f);

    // The artwork has the wolf in the lower-left and "YOU LOSE"
    // in the upper-left.  All our text sits on the right half so
    // it doesn't overlap the illustration.

    // ── Stats (right side, with backing) ─────────────────────────
    softTextBacking(1100, 620, 720, 220);

    char buf[64];
    sprintf(buf, "Final Score:  %d points", s->score);
    glColor3f(1.0f, 0.98f, 0.85f);
    drawTextAt(1140, 800, buf, GLUT_BITMAP_TIMES_ROMAN_24);

    sprintf(buf, "You reached Level %d / 3", s->currentLevel);
    glColor3f(1.0f, 0.92f, 0.78f);
    drawTextAt(1160, 740, buf, GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(1.0f, 0.78f, 0.78f);
    drawTextAt(1140, 670,
               "An enemy caught the rat. Try again?",
               GLUT_BITMAP_HELVETICA_18);

    // ── Pulsing prompt (right side, with backing) ────────────────
    softTextBacking(1100, 380, 720, 110);

    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1.0f, 0.98f, 0.85f, 0.85f + 0.15f * pulse);
    drawTextAt(1130, 450,
               "Press ENTER to play again",
               GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.95f, 0.85f, 0.62f);
    drawTextAt(1240, 405,
               "ESC to return to menu",
               GLUT_BITMAP_HELVETICA_18);

    end2D();
}

// ================================================================
//  LEVEL COMPLETE SCREEN
// ================================================================
void _gamestate::drawLevelCompleteScreen(_Scene* s)
{
    begin2D(1920, 1080);

    drawTexturedBackdrop(s);

    // Green-tinted overlay to signal success
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
        glColor4f(0.05f, 0.30f, 0.10f, 0.30f); glVertex2f(0, 0);    glVertex2f(1920, 0);
        glColor4f(0.03f, 0.18f, 0.06f, 0.18f); glVertex2f(1920, 1080); glVertex2f(0, 1080);
    glEnd();

    // ── Title with drop shadow (warm green, with backing) ───────
    softTextBacking(400, 685, 1120, 140);

    char title[64];
    sprintf(title, "LEVEL %d COMPLETE", s->currentLevel);
    drawStrokeShadow(440, 740, 0.78f, title, 0.55f, 0.95f, 0.55f);

    // Decorative green underline
    glColor4f(0.65f, 1.0f, 0.65f, 0.95f);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(440, 695); glVertex2f(1480, 695);
    glEnd();
    glLineWidth(1.0f);

    // ── Score (with backing) ─────────────────────────────────────
    softTextBacking(640, 580, 640, 50);

    char sbuf[64];
    sprintf(sbuf, "Score:  %d points", s->score);
    glColor3f(1.0f, 0.98f, 0.85f);
    drawTextAt(770, 600, sbuf, GLUT_BITMAP_TIMES_ROMAN_24);

    if (s->currentLevel < 3) {
        // Soft backing under the next-level info
        softTextBacking(440, 480, 1040, 90);

        char nbuf[128];
        sprintf(nbuf, "Next  -  Level %d  -  %d foods  -  %d dog%s + the cat",
                s->currentLevel+1, s->FOOD_PER_LEVEL[s->currentLevel],
                s->DOGS_PER_LEVEL[s->currentLevel],
                s->DOGS_PER_LEVEL[s->currentLevel] == 1 ? "" : "s");
        glColor3f(0.95f, 1.00f, 0.92f);
        drawTextAt(540, 540, nbuf, GLUT_BITMAP_HELVETICA_18);

        char pbuf[64];
        sprintf(pbuf, "Possible bonus:  +%d points", s->FOOD_PER_LEVEL[s->currentLevel] * 10);
        glColor3f(0.65f, 1.00f, 0.68f);
        drawTextAt(740, 500, pbuf, GLUT_BITMAP_HELVETICA_18);
    }

    // ── Pulsing prompt (with backing) ────────────────────────────
    softTextBacking(620, 340, 680, 50);

    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1.0f, 0.98f, 0.85f, 0.85f + 0.15f * pulse);
    drawTextAt(700, 360, "Press ENTER to continue",
               GLUT_BITMAP_TIMES_ROMAN_24);

    end2D();
}

// ================================================================
//  WIN SCREEN
// ================================================================
void _gamestate::drawWinScreen(_Scene* s)
{
    begin2D(1920, 1080);

    // ── Win artwork (YOU WIN! title baked into the art) ──────────
    // The art shows the rat hugging food in the centre with the
    // title at the top.  Body text sits below with a backing.
    drawCustomBackdrop(s->winBg, 0.10f);

    // ── Body (centred under the artwork's character, with backing) ─
    softTextBacking(360, 100, 1200, 250);

    glColor3f(1.0f, 0.98f, 0.85f);
    drawTextAt(560, 320,
               "You collected all the food across all 3 levels!",
               GLUT_BITMAP_TIMES_ROMAN_24);

    char sbuf[64];
    sprintf(sbuf, "Final Score:  %d points", s->score);
    glColor3f(1.0f, 0.92f, 0.30f);
    drawTextAt(700, 270, sbuf, GLUT_BITMAP_TIMES_ROMAN_24);

    glColor3f(1.0f, 0.92f, 0.78f);
    drawTextAt(580, 220,
               "The mouse lives to eat another day. Congratulations!",
               GLUT_BITMAP_HELVETICA_18);

    // ── Pulsing prompt ──────────────────────────────────────────
    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1.0f, 0.98f, 0.85f, 0.85f + 0.15f * pulse);
    drawTextAt(680, 160, "Press ENTER to play again",
               GLUT_BITMAP_TIMES_ROMAN_24);

    end2D();
}

// ================================================================
//  BOSS INTRO OVERLAY  –  shown for 3 seconds at start of Level 3.
//
//  Draws the gameplay scene normally (no pause), then layers a
//  Dishonored-themed full-screen splash over it that fades out
//  in the last 0.6 seconds.
// ================================================================
void _gamestate::drawBossIntro(_Scene* s)
{
    if (s->bossIntroTimer <= 0.0f) return;

    begin2D(1920, 1080);

    // Total duration is 3.0s.  Fade IN for 0.4s, hold, fade OUT for 0.6s.
    float t      = 3.0f - s->bossIntroTimer;          // 0..3
    float fadeIn = (t < 0.4f) ? (t / 0.4f) : 1.0f;
    float fadeOut = (s->bossIntroTimer < 0.6f) ? (s->bossIntroTimer / 0.6f) : 1.0f;
    float alpha = fadeIn * fadeOut;
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;

    // ── Black backdrop with deep red tint ────────────────────────
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
        glColor4f(0.04f, 0.02f, 0.02f, 0.92f * alpha); glVertex2f(0, 0);    glVertex2f(1920, 0);
        glColor4f(0.18f, 0.04f, 0.04f, 0.92f * alpha); glVertex2f(1920, 1080); glVertex2f(0, 1080);
    glEnd();

    // ── Procedural scratches on top for atmosphere ───────────────
    unsigned int u_val = 99887766u;
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < 80; i++) {
        float x = frand(u_val) * 1920.0f;
        float y = frand(u_val) * 1080.0f;
        float ang = frand(u_val) * 6.2832f;
        float len = 8.0f + frand(u_val) * 32.0f;
        float a   = (0.10f + frand(u_val) * 0.18f) * alpha;
        glColor4f(0.95f, 0.78f, 0.62f, a);
        glVertex2f(x, y);
        glVertex2f(x + cosf(ang) * len, y + sinf(ang) * len);
    }
    glEnd();

    // ── Title (top, drop shadow, deep blood red) ─────────────────
    // Animate slight scale-up during fade-in so it feels punchy.
    float scaleAnim = 0.85f + 0.10f * (1.0f - fadeIn);   // 0.95 -> 0.85
    float yPos = 700.0f;

    // Use drawStrokeShadow but with custom alpha via colour
    // Shadow first
    glLineWidth(5.0f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.85f * alpha);
    glPushMatrix();
    glTranslatef(440 + 4, yPos - 4, 0); glScalef(scaleAnim, scaleAnim, 1);
    for (const char* c = "BOSS LEVEL"; *c; c++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();
    // Main "BOSS LEVEL" in bright red
    glLineWidth(5.5f);
    glColor4f(1.0f, 0.20f, 0.15f, alpha);
    glPushMatrix();
    glTranslatef(440, yPos, 0); glScalef(scaleAnim, scaleAnim, 1);
    for (const char* c = "BOSS LEVEL"; *c; c++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();

    // Decorative red underline
    glColor4f(1.0f, 0.20f, 0.15f, 0.85f * alpha);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(440, yPos - 30); glVertex2f(1480, yPos - 30);
    glEnd();
    glLineWidth(1.0f);

    // ── Subtitle — "REVENGE OF THE FOOD" (cream colour) ──────────
    float subY = 540.0f;
    glLineWidth(4.0f);
    glColor4f(0.0f, 0.0f, 0.0f, 0.85f * alpha);
    glPushMatrix();
    glTranslatef(380 + 4, subY - 4, 0); glScalef(0.55f, 0.55f, 1);
    for (const char* c = "REVENGE OF THE FOOD"; *c; c++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();
    glLineWidth(4.5f);
    glColor4f(0.95f, 0.92f, 0.78f, alpha);
    glPushMatrix();
    glTranslatef(380, subY, 0); glScalef(0.55f, 0.55f, 1);
    for (const char* c = "REVENGE OF THE FOOD"; *c; c++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();
    glLineWidth(1.0f);

    // ── Flavour text ─────────────────────────────────────────────
    glColor4f(0.85f, 0.78f, 0.62f, alpha);
    drawTextAt(660, 430,
               "More food. More dogs. No mercy.",
               GLUT_BITMAP_TIMES_ROMAN_24);

    // Pulsing prompt at the bottom
    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.005f);
    glColor4f(0.95f, 0.20f, 0.15f, alpha * (0.5f + 0.5f * pulse));
    drawTextAt(810, 290,
               "F I N A L   L E V E L",
               GLUT_BITMAP_TIMES_ROMAN_24);

    end2D();
}

// ================================================================
//  MEGA-HIT BANNER  –  hit_text1.png shown at top of screen for
//                     1.5s after dashing into food.
//
//  Animates with a quick scale-pop on enter and a fade-out on the
//  last 0.4 seconds.  Sits high in the frame so it doesn't cover
//  gameplay, and uses the textured PNG (yellow MEGA-HIT! image).
// ================================================================
void _gamestate::drawMegaHitBanner(_Scene* s)
{
    if (s->megaHitTimer <= 0.0f || !s->megaHitTex) return;

    begin2D(1920, 1080);

    // Compute fade/scale from elapsed time
    float total = 1.5f;
    float t     = total - s->megaHitTimer;             // 0..1.5 elapsed
    float fadeIn  = (t < 0.15f) ? (t / 0.15f) : 1.0f;
    float fadeOut = (s->megaHitTimer < 0.4f) ? (s->megaHitTimer / 0.4f) : 1.0f;
    float alpha   = fadeIn * fadeOut;
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;

    // Pop-in scale: starts at 1.4, settles to 1.0
    float scaleAnim = 1.0f + 0.4f * (1.0f - fadeIn);

    // Image is 701 × 433.  We want it centered horizontally near the top,
    // ~340 wide on screen.
    const float baseW = 600.0f;
    const float baseH = 600.0f * (433.0f / 701.0f);    // preserve aspect
    float w = baseW * scaleAnim;
    float h = baseH * scaleAnim;
    float cx = 960.0f;                       // screen center X
    float cy = 950.0f;                       // near top
    float x = cx - w * 0.5f;
    float y = cy - h * 0.5f;

    // Draw the PNG with alpha
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    s->megaHitTex->BindTex();
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(x,     y);
        glTexCoord2f(1, 1); glVertex2f(x + w, y);
        glTexCoord2f(1, 0); glVertex2f(x + w, y + h);
        glTexCoord2f(0, 0); glVertex2f(x,     y + h);
    glEnd();
    glPopAttrib();

    end2D();
}

// ================================================================
//  PAUSE SCREEN
// ================================================================
// ================================================================
//  PAUSE MENU — 4 buttons: Save / Continue / Controls / Exit
//  Each is mouse-clickable AND has a number-key shortcut (1-4).
// ================================================================
static const char* PAUSE_LABELS[4] = {
    "1.  Save Game",
    "2.  Continue",
    "3.  Game Controls",
    "4.  Exit to Menu"
};

// Pause-menu strip geometry – matches landing menu (right-aligned)
static const float PMENU_X = 1190.0f;
static const float PMENU_Y = 380.0f;        // bottom-Y of LOWEST strip
static const float PMENU_W = 640.0f;
static const float PMENU_H = 64.0f;
static const float PMENU_GAP = 20.0f;

// Returns 0..3 if mouse is over a button, else -1
int _gamestate::pauseHoveredButton(_Scene* s)
{
    float mx = (float)s->mouseX;
    float my = (float)s->mouseY;
    if (mx < PMENU_X || mx > PMENU_X + PMENU_W) return -1;
    for (int i = 0; i < 4; i++) {
        // Buttons are stacked top-to-bottom: index 0 highest
        float by = PMENU_Y + (3 - i) * (PMENU_H + PMENU_GAP);
        if (my >= by && my <= by + PMENU_H) return i;
    }
    return -1;
}

void _gamestate::drawPauseScreen(_Scene* s)
{
    begin2D(1920, 1080);

    // ── Pause artwork backdrop ──────────────────────────────────
    // Soraya's pause illustration sits over the (now hidden)
    // gameplay viewport.  A light dark overlay (12%) keeps text
    // readable without washing out the rat-in-armchair scene.
    drawCustomBackdrop(s->pauseBg, 0.12f);

    // ── Title (top, centered above the strips) ──────────────────
    softTextBacking(800, 770, 360, 130);
    drawStrokeShadow(840, 800, 0.85f, "PAUSED", 0.95f, 0.92f, 0.78f);

    // Decorative line under the title
    glColor4f(0.85f, 0.78f, 0.50f, 0.6f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(840, 770); glVertex2f(1180, 770);
    glEnd();
    glLineWidth(1.0f);

    // ── 4 right-aligned torn-strip buttons ──────────────────────
    int hov = pauseHoveredButton(s);
    for (int i = 0; i < 4; i++) {
        float by = PMENU_Y + (3 - i) * (PMENU_H + PMENU_GAP);
        bool hovered = (i == hov);

        drawTornStrip(PMENU_X, by, PMENU_W, PMENU_H, hovered);

        const char* lbl = PAUSE_LABELS[i];
        int textPx = 0;
        for (const char* c = lbl; *c; c++)
            textPx += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, *c);
        if (hovered) glColor3f(0.05f, 0.05f, 0.05f);
        else         glColor3f(0.92f, 0.88f, 0.78f);
        drawTextAt(PMENU_X + PMENU_W - textPx - 36, by + 24,
                   lbl, GLUT_BITMAP_TIMES_ROMAN_24);
    }

    // Hint text below the strips (with soft backing)
    softTextBacking(60, 75, 600, 60);

    glColor3f(1.0f, 0.92f, 0.55f);
    drawTextAt(75, 110,
               "Click a button or press the number key (1-4)",
               GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.85f, 0.82f, 0.65f);
    drawTextAt(75, 85, "ESC also continues the game.",
               GLUT_BITMAP_HELVETICA_18);

    end2D();
}

// ================================================================
//  IN-GAME HELP OVERLAY (shown by 'H' or Pause→Controls)
// ================================================================
void _gamestate::drawHelpInGame(_Scene* s)
{
    begin2D(1920, 1080);

    // Heavy darken over gameplay so the strips read clearly
    fillRect(0, 0, 1920, 1080, 0, 0, 0, 0.82f);

    // Some grunge specks on top of the dimmed scene for atmosphere
    unsigned int u_val = 33333u;
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < 50; i++) {
        float x = frand(u_val) * 1920.0f;
        float y = frand(u_val) * 1080.0f;
        float ang = frand(u_val) * 6.2832f;
        float len = 8.0f + frand(u_val) * 24.0f;
        float a   = 0.06f + frand(u_val) * 0.10f;
        glColor4f(0.85f, 0.78f, 0.62f, a);
        glVertex2f(x, y);
        glVertex2f(x + cosf(ang) * len, y + sinf(ang) * len);
    }
    glEnd();

    // ── Title (top, large, with backing) ────────────────────────
    softTextBacking(340, 850, 760, 155);
    drawStrokeShadow(360, 920, 0.85f, "GAME CONTROLS", 0.95f, 0.92f, 0.78f);

    glColor3f(1.0f, 0.95f, 0.68f);
    drawTextAt(380, 870, "Mid-game reference",
               GLUT_BITMAP_TIMES_ROMAN_24);

    glColor4f(1.0f, 0.92f, 0.55f, 0.85f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(360, 855); glVertex2f(960, 855);
    glEnd();
    glLineWidth(1.0f);

    // ── Right-side info strips: key + description per row ───────
    const float SX = 1010.0f;
    const float SW = 820.0f;
    const float SH = 50.0f;
    const float SGAP = 10.0f;
    const float SY_TOP = 800.0f;

    struct Row { const char* k; const char* d; };
    Row rows[] = {
        { "WASD",        "Move the rat"             },
        { "Arrow Keys",  "Rotate camera (yaw/pitch)" },
        { "Right Mouse", "Drag to rotate camera"    },
        { "SPACE",       "Dash forward (jump anim)" },
        { "F",           "Fart - distract enemies"  },
        { "H",           "Show / hide this help"    },
        { "ESC",         "Open pause menu"          }
    };
    int n = sizeof(rows) / sizeof(rows[0]);
    for (int i = 0; i < n; i++) {
        float by = SY_TOP - i * (SH + SGAP);
        drawTornStrip(SX, by, SW, SH, false);
        glColor3f(1.0f, 0.85f, 0.30f);
        drawTextAt(SX + 24, by + 18, rows[i].k, GLUT_BITMAP_TIMES_ROMAN_24);
        glColor3f(0.92f, 0.88f, 0.78f);
        drawTextAt(SX + 220, by + 18, rows[i].d, GLUT_BITMAP_HELVETICA_18);
    }

    // ── Goal section (left side, with soft backing) ─────────────
    softTextBacking(45, 555, 630, 200);   // GOAL header + 3 lines

    glLineWidth(3.0f); glColor3f(1.0f, 0.95f, 0.78f);
    drawStroke(80, 700, 0.36f, "GOAL");
    glLineWidth(1.0f);

    glColor4f(1.0f, 0.92f, 0.50f, 0.85f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(80, 680); glVertex2f(360, 680);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(1.0f, 0.98f, 0.92f);
    drawTextAt(80, 640,
               "Push food into the mouse hole.",
               GLUT_BITMAP_HELVETICA_18);
    drawTextAt(80, 608,
               "Avoid the cats & dogs.",
               GLUT_BITMAP_HELVETICA_18);
    drawTextAt(80, 576,
               "Each food collected = +10 points.",
               GLUT_BITMAP_HELVETICA_18);

    // ── Bottom hint (with backing) ───────────────────────────────
    softTextBacking(60, 95, 350, 35);

    float pulse = 0.5f + 0.5f * sinf((float)glutGet(GLUT_ELAPSED_TIME) * 0.003f);
    glColor4f(1.0f, 0.92f, 0.55f, 0.85f + 0.15f * pulse);
    drawTextAt(80, 110, "Press ESC or H to close",
               GLUT_BITMAP_HELVETICA_18);

    end2D();
}

// ================================================================
//  SLOT MENU – Save (Pause→Save) and Load (Landing→Load)
//  Shows 3 numbered slots; click or press 1/2/3 to pick.
//  ESC returns to caller (pause menu / landing).
// ================================================================

// Slot button geometry
static const float SLOT_X = 1190.0f;     // right-aligned to match menu strips
static const float SLOT_W = 640.0f;
static const float SLOT_H = 100.0f;
static const float SLOT_TOP_Y = 660.0f;   // top button's bottom Y
static const float SLOT_GAP = 24.0f;

// Returns 1..3 if mouse over a slot, else 0
int _gamestate::slotHovered(_Scene* s)
{
    float mx = (float)mouseX;
    float my = (float)mouseY;
    if (mx < SLOT_X || mx > SLOT_X + SLOT_W) return 0;
    for (int i = 0; i < 3; i++) {
        float by = SLOT_TOP_Y - i * (SLOT_H + SLOT_GAP);
        if (my >= by && my <= by + SLOT_H) return i + 1;
    }
    return 0;
}

void _gamestate::drawSlotMenu(_Scene* s, const char* title)
{
    begin2D(1920, 1080);

    drawTexturedBackdrop(s);

    // ── Title (top, large stroke text, with backing) ────────────
    softTextBacking(340, 850, 760, 155);
    drawStrokeShadow(360, 920, 0.85f, title, 0.95f, 0.92f, 0.78f);

    // Subtitle hint
    glColor3f(1.0f, 0.95f, 0.68f);
    drawTextAt(380, 870, "Pick a slot below",
               GLUT_BITMAP_TIMES_ROMAN_24);

    // Decorative gold underline beneath title
    glColor4f(1.0f, 0.92f, 0.55f, 0.85f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(360, 855); glVertex2f(960, 855);
    glEnd();
    glLineWidth(1.0f);

    // ── 3 right-aligned torn-strip slot buttons ─────────────────
    int hov = slotHovered(s);
    for (int i = 0; i < _savegame::NUM_SLOTS; i++) {
        int slot = i + 1;
        SlotInfo info = _savegame::peek(slot);
        float by = SLOT_TOP_Y - i * (SLOT_H + SLOT_GAP);
        bool hovered = (slot == hov);

        drawTornStrip(SLOT_X, by, SLOT_W, SLOT_H, hovered);

        // Slot number (large stroke text on left of strip)
        char numbuf[8]; sprintf(numbuf, "%d", slot);
        glLineWidth(3.5f);
        if (hovered) glColor3f(0.05f, 0.05f, 0.05f);
        else         glColor3f(0.92f, 0.88f, 0.78f);
        drawStroke(SLOT_X + 30, by + 28, 0.55f, numbuf);
        glLineWidth(1.0f);

        // Slot info text – right-aligned within the strip
        if (info.exists) {
            char buf[64];
            sprintf(buf, "Level %d   -   Score: %d", info.level, info.score);
            int bufPx = 0;
            for (const char* c = buf; *c; c++)
                bufPx += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, *c);
            if (hovered) glColor3f(0.05f, 0.05f, 0.05f);
            else         glColor3f(0.92f, 0.88f, 0.78f);
            drawTextAt(SLOT_X + SLOT_W - bufPx - 36, by + 60,
                       buf, GLUT_BITMAP_TIMES_ROMAN_24);

            const char* hint = "click to use this slot";
            int hintPx = 0;
            for (const char* c = hint; *c; c++)
                hintPx += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
            if (hovered) glColor3f(0.20f, 0.20f, 0.18f);
            else         glColor3f(0.55f, 0.52f, 0.40f);
            drawTextAt(SLOT_X + SLOT_W - hintPx - 36, by + 28,
                       hint, GLUT_BITMAP_HELVETICA_18);
        } else {
            const char* lbl  = "[ Empty Slot ]";
            int lblPx = 0;
            for (const char* c = lbl; *c; c++)
                lblPx += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, *c);
            if (hovered) glColor3f(0.20f, 0.20f, 0.18f);
            else         glColor3f(0.55f, 0.52f, 0.50f);
            drawTextAt(SLOT_X + SLOT_W - lblPx - 36, by + 60,
                       lbl, GLUT_BITMAP_TIMES_ROMAN_24);

            const char* hint = "click to use this slot";
            int hintPx = 0;
            for (const char* c = hint; *c; c++)
                hintPx += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);
            if (hovered) glColor3f(0.30f, 0.30f, 0.28f);
            else         glColor3f(0.40f, 0.38f, 0.34f);
            drawTextAt(SLOT_X + SLOT_W - hintPx - 36, by + 28,
                       hint, GLUT_BITMAP_HELVETICA_18);
        }
    }

    // ── Bottom-left hint text (with backing) ────────────────────
    softTextBacking(60, 75, 540, 60);

    glColor3f(1.0f, 0.92f, 0.55f);
    drawTextAt(75, 110,
               "Click a slot or press 1, 2, or 3",
               GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.85f, 0.82f, 0.65f);
    drawTextAt(75, 85, "ESC to cancel",
               GLUT_BITMAP_HELVETICA_18);

    end2D();
}

void _gamestate::draw(_Scene* s)
{
    switch (s->gameState)
    {
        case GS_LANDING:           drawLandingScreen(s); break;
        case GS_LANDING_INFO:      drawLandingInfo(s); break;
        case GS_LANDING_CREDITS:   drawCredits(s); break;
        case GS_GAME_OVER:         drawGameOverScreen(s); break;
        case GS_LEVEL_COMPLETE:    drawLevelCompleteScreen(s); break;
        case GS_WIN:               drawWinScreen(s); break;
        case GS_PAUSED:            drawPauseScreen(s); break;
        case GS_HELP_INGAME:       drawHelpInGame(s); break;
        case GS_SAVE_MENU:         drawSlotMenu(s, "Save Game"); break;
        case GS_LOAD_MENU:         drawSlotMenu(s, "Load Game"); break;
        default: break;
    }

    // overlays
    if (s->bossIntroTimer > 0)
        drawBossIntro(s);

    if (s->megaHitTimer > 0)
        drawMegaHitBanner(s);
}
