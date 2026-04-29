// ================================================================
//  _ui.cpp  –  In-game HUD with high-contrast gold-on-black design
// ================================================================
#include "_ui.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <cstdio>
#include <cmath>

_ui::_ui()  {}
_ui::~_ui() {}

void _ui::draw(int width, int height,
               int score,
               float dashTimer, float dashCooldown, bool canDash,
               float fartTimer, float fartCooldown, bool canFart)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ── Solid black backing ─────────────────────────────────────
    glColor4f(0.0f, 0.0f, 0.0f, 0.94f);
    drawRect(10, height - 175, 280, 170);

    // ── Bright gold border ──────────────────────────────────────
    glColor4f(1.0f, 0.85f, 0.20f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(10,  height - 175); glVertex2f(290, height - 175);
    glVertex2f(290, height - 5);   glVertex2f(10,  height - 5);
    glEnd();
    glLineWidth(1.0f);

    // ── Score (large, bright gold) ──────────────────────────────
    char sbuf[32];
    sprintf(sbuf, "Score: %d", score);
    glColor3f(1.0f, 0.88f, 0.25f);
    drawText(20, height - 32, sbuf);

    // Gold divider line
    glColor4f(1.0f, 0.85f, 0.20f, 0.8f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(20, height - 50); glVertex2f(280, height - 50);
    glEnd();
    glLineWidth(1.0f);

    // ── Dash bar ─────────────────────────────────────────────────
    float dashPct = canDash ? 1.0f
                  : glm::clamp(1.0f - dashTimer/dashCooldown, 0.0f, 1.0f);

    // Label (bright gold)
    glColor3f(1.0f, 0.88f, 0.25f);
    drawText(20, height - 70, "Dash [SPACE]");
    // Status (gold for READY, dim gold for recharging)
    if (canDash) glColor3f(0.40f, 1.0f, 0.50f);   // green when ready
    else         glColor3f(0.85f, 0.55f, 0.20f);  // amber when recharging
    drawText(180, height - 70, canDash ? "READY" : "WAIT");

    // Bar background (dark)
    glColor4f(0.05f, 0.05f, 0.05f, 1.0f);
    drawRect(20, height - 95, 260, 14);
    // Bar fill
    if (canDash) glColor4f(0.20f, 0.85f, 0.30f, 1.0f);   // green fill
    else         glColor4f(0.85f, 0.55f, 0.10f, 1.0f);   // amber fill
    drawRect(20, height - 95, 260 * dashPct, 14);
    // Bar border (gold)
    glColor4f(1.0f, 0.85f, 0.20f, 0.9f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(20, height - 95);  glVertex2f(280, height - 95);
    glVertex2f(280, height - 81); glVertex2f(20, height - 81);
    glEnd();
    glLineWidth(1.0f);

    // ── Fart bar ─────────────────────────────────────────────────
    float fartPct = canFart ? 1.0f
                  : glm::clamp(1.0f - fartTimer/fartCooldown, 0.0f, 1.0f);

    glColor3f(1.0f, 0.88f, 0.25f);
    drawText(20, height - 120, "Fart [F]");
    if (canFart) glColor3f(0.40f, 1.0f, 0.50f);
    else         glColor3f(0.85f, 0.55f, 0.20f);
    drawText(180, height - 120, canFart ? "READY" : "WAIT");

    glColor4f(0.05f, 0.05f, 0.05f, 1.0f);
    drawRect(20, height - 145, 260, 14);
    if (canFart) glColor4f(0.20f, 0.85f, 0.30f, 1.0f);
    else         glColor4f(0.85f, 0.55f, 0.10f, 1.0f);
    drawRect(20, height - 145, 260 * fartPct, 14);
    glColor4f(1.0f, 0.85f, 0.20f, 0.9f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(20, height - 145);  glVertex2f(280, height - 145);
    glVertex2f(280, height - 131); glVertex2f(20, height - 131);
    glEnd();
    glLineWidth(1.0f);

    glDisable(GL_BLEND);
}

// ── Helpers ───────────────────────────────────────────────────────
void _ui::drawText(float x, float y, const std::string& text)
{
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}

void _ui::drawRect(float x, float y, float w, float h)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();
}
