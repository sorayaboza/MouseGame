#ifndef _UI_H
#define _UI_H

#pragma once
#include <string>

class _ui {
public:
    _ui();
    ~_ui();

    void draw(int width, int height,
              int score,
              float dashTimer, float dashCooldown, bool canDash,
              float fartTimer, float fartCooldown, bool canFart);

private:
    void drawText(float x, float y, const std::string& text);
    void drawRect(float x, float y, float w, float h);
};

#endif // _UI_H
