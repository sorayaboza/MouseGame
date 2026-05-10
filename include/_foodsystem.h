#ifndef _FOODSYSTEM_H
#define _FOODSYSTEM_H

#include <vector>
#include "_food.h"
#include "_player.h"
#include "_skybox.h"

class _foodsystem {
public:
    _foodsystem();
    ~_foodsystem();

    void init(_skyBox* skyRef);
    void update(float dt, float floorY);

    void handleCollisions(); // Food-to-food collisions
    void handlePlayerCollisions(_player* player, glm::vec3 playerMoveDir,
                                 bool playerIsDashing); // Food-to-player collisions

    // Set by handlePlayerCollisions: number of foods hit by the
    // dashing player this frame.  Scene reads it to spawn the
    // MEGA-HIT visual + SFX.
    int  megaHitsThisFrame;
    void checkFoodInHole(glm::vec3 holePos, float holeRadius, int& score, float dt);

    // Set by checkFoodInHole each call: number of foods that just hit
    // the hole this frame.  Scene reads it for the SFX trigger.
    int  foodCollectedThisFrame;
    void spawnFoods(int numFoods, int level);

    std::vector<_food*> foods;

private:
    _skyBox* sky;
};

#endif
