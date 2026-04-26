#include "_renderer.h"
#include <GL/glut.h>

_renderer::_renderer() {}
_renderer::~_renderer() {}

void _renderer::renderFrame(FrameRenderData& frame)
{
    drawPlayer(frame.player);

    for (auto& f : *frame.foods) { drawFood(f); }
    for (auto& f : *frame.farts) { drawFart(f); }

    drawMouseHole(frame.mouseHolePos, frame.mouseHoleRadius);
}


void _renderer::drawPlayer(const PlayerRenderData& p)
{
    glPushMatrix();

    glTranslatef(p.pos.x, p.pos.y, p.pos.z);

    // MD2 FIXES
    glRotatef(p.rotY, 0, 1, 0);
    glRotatef(-90, 1, 0, 0);

    glScalef(0.1f, 0.1f, 0.1f);

    p.model->Draw();

    glPopMatrix();
}

void _renderer::drawFood(const FoodRenderData& f)
{
    glPushMatrix();

    glTranslatef(f.pos.x, f.pos.y, f.pos.z);

    glRotatef(f.rotY, 0, 1, 0);
    glRotatef(-90, 1, 0, 0);

    glScalef(0.08f, 0.08f, 0.08f);

    f.model->Draw();

    glPopMatrix();
}

void _renderer::drawFart(const FartRenderData& f) {
    glPushMatrix();

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glTranslatef(f.pos.x, f.pos.y, f.pos.z);
    glColor3f(0.3f, 1.0f, 0.3f);
    glutSolidSphere(0.5, 10, 10);

    glPopAttrib();
    glPopMatrix();
}

void _renderer::drawMouseHole(const glm::vec3& pos,float radius) {
    glPushMatrix();

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glTranslatef(pos.x, pos.y, pos.z);
    glColor3f(0.05f, 0.05f, 0.05f); // dark hole

    GLUquadric* quad = gluNewQuadric();

    glRotatef(90, 0, 0, 1); // Rotate so cylinder faces forward (out of wall)

    float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    // Pulse between ~90% and 110%
    float pulse = 1.0f + 0.1f * sin(time * 3.0f);

    float visualRadius = radius * pulse;

    glScalef(visualRadius, visualRadius, visualRadius * 0.2f);

    // Draw cylinder (radius, radius, height, slices, stacks)
    gluCylinder(quad, 1.0f, 1.0f, 1.0f, 32, 32);
    gluDeleteQuadric(quad);

    glPushMatrix();

    // move slightly inside the hole
    glTranslatef(0, 0, 0.5f);

    // slightly smaller inner disk
    glScalef(visualRadius * 0.1f, visualRadius * 0.1f, 0.1f);

    // darker center
    glColor3f(0.0f, 0.0f, 0.0f);

    GLUquadric* innerQuad = gluNewQuadric();
    gluDisk(innerQuad, 0.0f, 1.0f, 32, 1);
    gluDeleteQuadric(innerQuad);

    glPopMatrix();

    glPopAttrib();
    glPopMatrix();
}
