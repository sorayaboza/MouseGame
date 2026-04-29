#include "_abilities.h"
#include <windows.h>
#include <glm/glm.hpp>
#include <algorithm>

_abilities::_abilities() {
    spaceWasDown = false;
    fWasDown = false;

    // DASH
    isDashingState = false;
    canDash = true;

    dashDuration = 0.40f;     // long enough for a full JUMP animation cycle
    dashTimer = 0.0f;

    dashCooldown = 2.0f;
    dashCooldownTimer = 0.0f;

    dashVelocity = glm::vec3(0);

    // FART
    canFart = true;
    fartCooldown = 2.0f;
    fartTimer = 0.0f;
    fartAnimTimer = 0.0f;
}

void _abilities::handleInput(glm::vec3 moveDir, glm::vec3 playerPos) {
    bool spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000);

    if (spaceDown && !spaceWasDown && canDash) {

    glm::vec3 dir = moveDir;

    if (glm::length(dir) == 0.0f)
        dir = glm::vec3(0,0,-1);
    else
        dir = glm::normalize(dir);


        dashVelocity = glm::normalize(dir) * 120.0f;

        isDashingState = true;
        canDash = false;
        dashTimer = dashDuration;
    }

    spaceWasDown = spaceDown;

    bool fDown = (GetAsyncKeyState('F') & 0x8000);

    if (fDown && !fWasDown && canFart) {
        Fart f;
        f.life = 3.0f;
        f.pos = playerPos;

        farts.push_back(f);

        canFart = false;
        fartTimer = fartCooldown;
        fartAnimTimer = 1.4f;   // play full ATTACK animation cycle
    }


    fWasDown = fDown;
}

void _abilities::update(float dt, glm::vec3& playerPos, glm::vec3 moveDir) {

    // DASH
    if (isDashingState) {

        playerPos += dashVelocity * dt;

        dashTimer -= dt;

        if (dashTimer <= 0.0f) {
            isDashingState = false;
            dashCooldownTimer = dashCooldown;
        }
    }

    // cooldown
    if (!canDash) {
        dashCooldownTimer -= dt;

        if (dashCooldownTimer <= 0.0f) {
            canDash = true;
        }
    }

    // FART UPDATE
    for (auto &f : farts) {
        f.life -= dt;
    }

    farts.erase(
        std::remove_if(farts.begin(), farts.end(),
            [](Fart &f){ return f.life <= 0.0f; }),
        farts.end()
    );

    if (!canFart) {
        fartTimer -= dt;
        if (fartTimer <= 0.0f) {
            canFart = true;
        }
    }
    if (fartAnimTimer > 0.0f) fartAnimTimer -= dt;
}

bool _abilities::isFartAnimating() {
    return fartAnimTimer > 0.0f;
}

bool _abilities::isDashing() {
    return isDashingState;
}
