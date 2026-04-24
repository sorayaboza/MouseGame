#include "_ui.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>

_ui::_ui() {}
_ui::~_ui() {}

void _ui::draw(int width, int height,
               int score,
               float dashTimer, float dashCooldown, bool canDash,
               float fartTimer, float fartCooldown, bool canFart)
{
    // --- SWITCH TO 2D ---
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    // =========================
    // SCORE
    // =========================
    glColor3f(1,1,1);
    drawText(20, height - 40, "Score: " + std::to_string(score));

    // =========================
    // DASH BAR
    // =========================
    float dashPercent = 1.0f - (dashTimer / dashCooldown);
    dashPercent = glm::clamp(dashPercent, 0.0f, 1.0f);

    // background
    glColor3f(0.2f,0.2f,0.2f);
    drawRect(20, height - 80, 200, 20);

    // fill
    if (!canDash) glColor3f(1.0f,0.3f,0.3f);
    else          glColor3f(0.2f,0.8f,1.0f);

    drawRect(20, height - 80, 200 * dashPercent, 20);

    drawText(20, height - 100, "Dash");

    // =========================
    // FART BAR
    // =========================
    float fartPercent = 1.0f - (fartTimer / fartCooldown);
    fartPercent = glm::clamp(fartPercent, 0.0f, 1.0f);

    glColor3f(0.2f,0.2f,0.2f);
    drawRect(20, height - 120, 200, 20);

    if (!canFart) glColor3f(1.0f,0.3f,0.3f);
    else          glColor3f(0.4f,1.0f,0.4f);

    drawRect(20, height - 120, 200 * fartPercent, 20);

    drawText(20, height - 140, "Fart");

    // --- RESTORE ---
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// ---------------- HELPERS ----------------

void _ui::drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void _ui::drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
    glEnd();
}
