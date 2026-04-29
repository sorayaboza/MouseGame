// ================================================================
//  _renderer.cpp  –  MD2 player, billboard food, fart, hole
//
//  Critical: every render function uses glPushAttrib/glPopAttrib
//  to fully save and restore the GL state, including TEXTURE_BIT,
//  ENABLE_BIT and CURRENT_BIT.  This prevents one render call from
//  leaking state into the next and causing missing textures.
// ================================================================
#include "_renderer.h"
#include <GL/glut.h>
#include <cmath>

_renderer::_renderer()  {}
_renderer::~_renderer() {}

void _renderer::renderFrame(FrameRenderData& frame)
{
    drawPlayer(frame.player);
    for (auto& f : *frame.foods) drawFoodBillboard(f, frame.cameraPos);
    for (auto& f : *frame.farts) drawFart(f);
    drawMouseHole(frame.mouseHolePos, frame.mouseHoleRadius);
}

// ================================================================
//  Player MD2 (the rat)
//  Y offset keeps the feet on the floor, not the model centre.
// ================================================================
void _renderer::drawPlayer(const PlayerRenderData& p)
{
    // Q2 models are ~56 units tall.  After scale 0.20 → 11.2 units.
    // Half-height is 5.6 — translate UP by that so feet touch floor.
    const float modelScale     = 0.20f;
    const float halfModelHeight = 56.0f * modelScale * 0.5f;

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);

    glPushMatrix();
        glTranslatef(p.pos.x, p.pos.y + halfModelHeight, p.pos.z);
        glRotatef(p.rotY, 0, 1, 0);
        glRotatef(-90, 1, 0, 0);
        glScalef(modelScale, modelScale, modelScale);
        p.model->Draw();
    glPopMatrix();

    glPopAttrib();
}

// ================================================================
//  Food billboard
//  glPushAttrib(GL_TEXTURE_BIT) ensures the texture binding is
//  fully saved & restored – fixes the food disappearing after MD2
//  rendering binds its own texture.
// ================================================================
void _renderer::drawFoodBillboard(const FoodRenderData& f,
                                   const glm::vec3& camPos)
{
    glm::vec3 toCam(camPos.x - f.pos.x, 0.0f, camPos.z - f.pos.z);
    float lenSq = toCam.x*toCam.x + toCam.z*toCam.z;
    if (lenSq < 0.0001f) toCam = glm::vec3(0,0,1);
    else                 toCam = glm::normalize(toCam);

    glm::vec3 right(-toCam.z, 0.0f, toCam.x);
    float halfW = f.size * 0.5f;
    float h     = f.size;

    // Position the billboard so its bottom sits on the floor.
    // physics.pos is at floor + collisionRadius (approximate sphere center),
    // so we drop the bottom by collisionRadius to land on the floor.
    float bottomDrop = f.size * 0.42f;
    glm::vec3 base   = f.pos - glm::vec3(0, bottomDrop, 0);

    glm::vec3 bl = base - right*halfW;
    glm::vec3 br = base + right*halfW;
    glm::vec3 tr = base + right*halfW + glm::vec3(0,h,0);
    glm::vec3 tl = base - right*halfW + glm::vec3(0,h,0);

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT |
                 GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Hard reset the state that can cause "black food" symptoms:
    glDisable(GL_LIGHTING);            // ensure glColor controls colour
    glDisable(GL_FOG);                 // fog can darken food
    glDisable(GL_COLOR_MATERIAL);      // stop any residual material lighting
    glActiveTexture(GL_TEXTURE0);      // bind to unit 0 explicitly
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);     // softer cutoff – allows semi-transparent edges

    // Bind food texture (white tint so PNG colours show through)
    if (f.texture) f.texture->BindTex();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex3f(bl.x, bl.y, bl.z);
        glTexCoord2f(1, 1); glVertex3f(br.x, br.y, br.z);
        glTexCoord2f(1, 0); glVertex3f(tr.x, tr.y, tr.z);
        glTexCoord2f(0, 0); glVertex3f(tl.x, tl.y, tl.z);
    glEnd();

    // Soft elliptical shadow under each food (on floor)
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glColor4f(0, 0, 0, 0.35f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(base.x, base.y - 0.1f, base.z);
    for (int i = 0; i <= 16; i++) {
        float a = i * 2.0f * 3.14159f / 16.0f;
        glVertex3f(base.x + cosf(a)*halfW*0.85f,
                   base.y - 0.1f,
                   base.z + sinf(a)*halfW*0.5f);
    }
    glEnd();

    glPopAttrib();
}

void _renderer::drawFart(const FartRenderData& f)
{
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glPushMatrix();
        glTranslatef(f.pos.x, f.pos.y + 1.5f, f.pos.z);
        glColor4f(0.40f, 0.95f, 0.30f, 0.55f);
        glutSolidSphere(2.0, 14, 14);
        glColor4f(0.55f, 1.00f, 0.50f, 0.30f);
        glutSolidSphere(3.5, 14, 14);
    glPopMatrix();

    glPopAttrib();
}

// ================================================================
//  Mouse hole – flat dark disc on floor + magnet-zone ring
// ================================================================
void _renderer::drawMouseHole(const glm::vec3& pos, float radius)
{
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_TEXTURE_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    float time  = (float)glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    float pulse = 1.0f + 0.06f * sinf(time * 3.0f);
    float r     = radius * pulse;

    // Outer dark disc
    glPushMatrix();
        glTranslatef(pos.x, pos.y - 0.45f, pos.z);
        glRotatef(-90, 1, 0, 0);
        glColor3f(0.05f, 0.04f, 0.03f);
        GLUquadric* q = gluNewQuadric();
        gluDisk(q, 0.0f, r, 32, 1);
        gluDeleteQuadric(q);
    glPopMatrix();

    // Inner blackness
    glPushMatrix();
        glTranslatef(pos.x, pos.y - 0.40f, pos.z);
        glRotatef(-90, 1, 0, 0);
        glColor3f(0, 0, 0);
        GLUquadric* q2 = gluNewQuadric();
        gluDisk(q2, 0.0f, r * 0.7f, 32, 1);
        gluDeleteQuadric(q2);
    glPopMatrix();

    // Pulsing yellow ring (visualises the magnet's pull radius)
    // Magnet radius is 18 in foodsystem; hole radius is 6, so ring at 3×.
    float ringRadius = radius * 3.0f;
    float ringAlpha  = 0.18f + 0.10f * sinf(time * 2.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.95f, 0.85f, 0.20f, ringAlpha);
    glLineWidth(2.5f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 64; i++) {
        float a = i * 2.0f * 3.14159f / 64.0f;
        glVertex3f(pos.x + cosf(a)*ringRadius,
                   pos.y - 0.35f,
                   pos.z + sinf(a)*ringRadius);
    }
    glEnd();
    glLineWidth(1.0f);

    glPopAttrib();
}
