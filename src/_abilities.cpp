/* ================================================================
  _abilities.cpp

  PURPOSE:
  This class manages the player's special abilities:

      1. DASH  (SPACE key)
         - Quickly launches the player forward
         - Uses cooldown timers
         - Triggers jump animation + sound effect

      2. FART ability (F key)
         - Creates temporary fart clouds
         - Enemies can detect these and become distracted
         - Uses cooldown timers
         - Triggers attack animation + sound effect

  HOW THE SYSTEM WORKS:

  INPUT PHASE:
      handleInput()
      - Detects keyboard presses
      - Starts abilities when allowed
      - Prevents repeated activation while holding a key

  UPDATE PHASE:
      update()
      - Moves the player during dash
      - Reduces cooldown timers
      - Removes expired fart clouds
      - Updates animation timers

  QUERY FUNCTIONS:
      isDashing()
      isFartAnimating()
      - Allow other systems to react to ability states

  RESET PHASE:
      clearAll()
      - Clears all temporary ability data when changing levels

  OTHER SYSTEM CONNECTIONS:

      _scene.cpp
          Calls handleInput() and update()

      _enemy.cpp
          Reads fart cloud positions to distract enemies

      _player / animations
          Uses ability states to select animations

      _sounds
          Uses trigger flags for dash/fart sound effects

 ================================================================ */

#include "_abilities.h"

#include <windows.h>       // GetAsyncKeyState()
#include <glm/glm.hpp>     // glm::vec3 math
#include <algorithm>       // std::remove_if



// Constructor
// Initializes all ability states, timers, cooldowns, and edge-trigger flags.
_abilities::_abilities() {

    // Tracks previous SPACE key state
    // Used for "edge detection" so abilities trigger ONCE
    // instead of repeating every frame while holding the key.
    spaceWasDown = false;

    fWasDown = false; // Tracks previous F key state



    // ============================================================
    // DASH INITIALIZATION
    // ============================================================

    // Player is NOT currently dashing
    isDashingState = false;

    // Dash starts available
    canDash = true;

    // Dash lasts 0.40 seconds
    // Long enough to play the full jump animation.
    dashDuration = 0.40f;

    // Remaining dash time
    dashTimer = 0.0f;

    // Cooldown before dash can be used again
    dashCooldown = 2.0f;

    // Countdown timer for cooldown
    dashCooldownTimer = 0.0f;

    // Current dash movement velocity
    dashVelocity = glm::vec3(0);



    // ============================================================
    // FART INITIALIZATION
    // ============================================================

    // Fart starts available
    canFart = true;

    // Cooldown duration
    fartCooldown = 2.0f;

    // Countdown timer
    fartTimer = 0.0f;

    // Animation timer for ATTACK animation
    fartAnimTimer = 0.0f;



    // ============================================================
    // SOUND EFFECT EDGE TRIGGERS
    //
    // These become TRUE for one frame when the ability activates.
    // _scene.cpp checks these and plays sound effects.
    // ============================================================

    dashJustTriggered = false;
    fartJustTriggered = false;
}



/* ================================================================
  handleInput()

  PURPOSE:
      Reads keyboard input and activates abilities.

  PARAMETERS:
      moveDir  -> current player movement direction
      playerPos -> current player position

  IMPORTANT:
      This function only STARTS abilities.
      Actual movement/timers happen in update().
 ================================================================ */
void _abilities::handleInput(glm::vec3 moveDir, glm::vec3 playerPos) {

    // ============================================================
    // DASH INPUT
    // ============================================================

    // Check whether SPACE is currently held down.
    //
    // GetAsyncKeyState(VK_SPACE)
    // returns keyboard state bits from Windows.
    //
    // 0x8000 means the key is currently pressed.
    bool spaceDown = (GetAsyncKeyState(VK_SPACE) & 0x8000);



    // ============================================================
    // DASH ACTIVATION
    //
    // Conditions:
    //   1. SPACE is pressed now
    //   2. SPACE was NOT pressed last frame
    //   3. Dash is available
    //
    // This creates an "edge trigger":
    // holding SPACE does not repeatedly dash.
    // ============================================================
    if (spaceDown && !spaceWasDown && canDash) {

        // Copy current movement direction
        glm::vec3 dir = moveDir;



        // ========================================================
        // If player is standing still,
        // dash forward by default.
        // ========================================================
        if (glm::length(dir) == 0.0f)

            // Forward direction
            dir = glm::vec3(0,0,-1);

        else

            // Normalize keeps movement direction length = 1
            dir = glm::normalize(dir);



        // ========================================================
        // Dash velocity
        //
        // Dash direction * dash speed
        // ========================================================
        dashVelocity = glm::normalize(dir) * 120.0f;



        // Dash is now active
        isDashingState = true;

        // Disable dash until cooldown finishes
        canDash = false;

        // Start dash duration countdown
        dashTimer = dashDuration;



        // ========================================================
        // SFX trigger
        //
        // _scene.cpp checks this flag and plays dash.mp3
        // ========================================================
        dashJustTriggered = true;
    }



    // Save current key state for next frame
    spaceWasDown = spaceDown;



    // ============================================================
    // FART INPUT
    // ============================================================

    // Check whether F is currently pressed
    bool fDown = (GetAsyncKeyState('F') & 0x8000);



    // ============================================================
    // FART ACTIVATION
    // ============================================================
    if (fDown && !fWasDown && canFart) {

        // Create a new fart cloud
        Fart f;

        // Fart exists for 3 seconds
        f.life = 3.0f;

        // Spawn fart at player's current position
        f.pos = playerPos;



        // Store fart in active fart list
        //
        // _enemy.cpp later checks this vector
        // to determine if enemies are distracted.
        farts.push_back(f);



        // Ability enters cooldown
        canFart = false;

        // Start cooldown timer
        fartTimer = fartCooldown;



        // ========================================================
        // Animation timer
        //
        // _scene.cpp checks this to play ATTACK animation.
        // ========================================================
        fartAnimTimer = 1.4f;



        // Trigger fart sound effect
        fartJustTriggered = true;
    }



    // Save F key state for next frame
    fWasDown = fDown;
}



/* ================================================================
  update()

  PURPOSE:
      Updates active abilities every frame.

  PARAMETERS:
      dt         -> delta time (frame time)
      playerPos  -> player's position (modifiable)
      moveDir    -> player movement direction

  RESPONSIBILITIES:
      - Move player while dashing
      - Reduce cooldown timers
      - Remove expired fart clouds
      - Update animation timers
 ================================================================ */
void _abilities::update(float dt,
                        glm::vec3& playerPos,
                        glm::vec3 moveDir) {

    // ============================================================
    // DASH UPDATE
    // ============================================================

    // Only update movement if dash is active
    if (isDashingState) {

        // ========================================================
        // Move player using velocity * delta time
        //
        // Classic frame-rate independent movement.
        // ========================================================
        playerPos += dashVelocity * dt;



        // Reduce remaining dash duration
        dashTimer -= dt;



        // ========================================================
        // End dash when timer finishes
        // ========================================================
        if (dashTimer <= 0.0f) {

            // Dash movement stops
            isDashingState = false;

            // Start cooldown timer
            dashCooldownTimer = dashCooldown;
        }
    }



    // ============================================================
    // DASH COOLDOWN UPDATE
    // ============================================================

    // Only count down if dash is unavailable
    if (!canDash) {

        // Reduce cooldown timer
        dashCooldownTimer -= dt;



        // Re-enable dash once cooldown finishes
        if (dashCooldownTimer <= 0.0f) {

            canDash = true;
        }
    }



    // ============================================================
    // FART CLOUD UPDATE
    // ============================================================

    // Reduce remaining life for every fart cloud
    for (auto &f : farts) {

        f.life -= dt;
    }



    // ============================================================
    // REMOVE EXPIRED FARTS
    //
    // remove_if() moves expired elements to the end.
    // erase() permanently deletes them from the vector.
    //
    // This is called the erase-remove idiom.
    // ============================================================
    farts.erase(

        std::remove_if(
            farts.begin(),
            farts.end(),

            // Lambda condition:
            // remove fart if its life expired
            [](Fart &f){
                return f.life <= 0.0f;
            }),

        farts.end()
    );



    // ============================================================
    // FART COOLDOWN UPDATE
    // ============================================================

    if (!canFart) {

        // Reduce cooldown timer
        fartTimer -= dt;



        // Re-enable fart ability
        if (fartTimer <= 0.0f) {

            canFart = true;
        }
    }



    // ============================================================
    // FART ANIMATION TIMER
    //
    // Used by player animation logic.
    // ============================================================
    if (fartAnimTimer > 0.0f)

        fartAnimTimer -= dt;
}



/* ================================================================
  isFartAnimating()

  PURPOSE:
      Returns TRUE while the fart animation timer is active.

  Used by:
      _scene.cpp animation logic

  Animation priority:
      ATTACK > JUMP > RUN > STAND
 ================================================================ */
bool _abilities::isFartAnimating() {

    return fartAnimTimer > 0.0f;
}



/* ================================================================
  clearAll()

  PURPOSE:
      Resets ALL temporary ability state.

  Used when:
      - starting a new level
      - resetting gameplay

  Prevents abilities/timers/farts from leaking between levels.
 ================================================================ */
void _abilities::clearAll() {

    // Remove all fart clouds
    farts.clear();



    // Stop active dash
    isDashingState = false;

    // Reset timers
    dashTimer = 0.0f;
    fartAnimTimer = 0.0f;
    fartTimer = 0.0f;



    // Re-enable abilities
    canDash = true;
    canFart = true;



    // Reset SFX flags
    dashJustTriggered = false;
    fartJustTriggered = false;



    // Stop dash movement
    dashVelocity = glm::vec3(0);
}



/* ================================================================
  isDashing()

  PURPOSE:
      Allows other systems to check whether the player
      is currently dashing.

  Used by:
      _scene.cpp
      animation logic
      collision systems
 ================================================================ */
bool _abilities::isDashing() {

    return isDashingState;
}
