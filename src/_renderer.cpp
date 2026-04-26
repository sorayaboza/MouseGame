#include "_renderer.h"
#include <GL/glut.h>

_renderer::_renderer() {}
_renderer::~_renderer() {}

void _renderer::renderFrame(FrameRenderData& frame)
{
    drawPlayer(frame.player);

    for (auto& f : *frame.foods) {
        drawFood(f);
    }
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
