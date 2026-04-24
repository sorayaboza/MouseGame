#ifndef _ABILITIES_H
#define _ABILITIES_H

#pragma once
#include<_common.h>
#include <glm/glm.hpp>
#include <vector>

struct Fart {
    glm::vec3 pos;
    float life;
};

class _abilities {
public:
    _abilities();

    void update(float dt, glm::vec3& playerPos, glm::vec3 moveDir);
    void handleInput(glm::vec3 moveDir, glm::vec3 playerPos);

    // DASH
    bool canDash;
    float dashCooldownTimer;
    float dashCooldown;
    bool isDashingState;

    // FART
    bool canFart;
    float fartTimer;
    float fartCooldown;
    std::vector<Fart> farts;

    // dash movement output
    bool isDashing();

private:
    // INPUT STATE
    bool spaceWasDown;
    bool fWasDown;

    float dashTimer;
    float dashDuration;

    glm::vec3 dashVelocity;

};

#endif // _ABILITIES_H
