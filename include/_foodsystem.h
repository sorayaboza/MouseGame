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
    void handlePlayerCollisions(_player* player, glm::vec3 playerMoveDir); // Food-to-player collisions
    void checkFoodInHole(glm::vec3 holePos, float holeRadius, int& score, float dt);
    void spawnFoods(int numFoods);

    std::vector<_food*> foods;

private:
    _skyBox* sky;
};

#endif
