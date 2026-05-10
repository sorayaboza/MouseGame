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
    float fartAnimTimer;        // counts down while ATTACK animation plays
    std::vector<Fart> farts;

    // dash movement output
    bool isDashing();
    bool isFartAnimating();

    // Wipe all transient effects (active fart clouds, dash velocity,
    // cooldowns).  Called between levels and after death so effects
    // from the previous run don't carry over.
    void clearAll();

    // ── Edge-trigger flags for sound effects ─────────────────
    // Set to true the frame Dash/Fart fires; the scene reads them
    // each frame to play the SFX, then resets to false via the
    // consumeSfxFlags() helper.
    bool dashJustTriggered;
    bool fartJustTriggered;
    void consumeSfxFlags() { dashJustTriggered = false; fartJustTriggered = false; }

private:
    // INPUT STATE
    bool spaceWasDown;
    bool fWasDown;

    float dashTimer;
    float dashDuration;

    glm::vec3 dashVelocity;

};

#endif // _ABILITIES_H
