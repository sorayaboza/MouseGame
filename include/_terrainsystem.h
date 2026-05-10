#ifndef _TERRAINSYSTEM_H
#define _TERRAINSYSTEM_H

#include <vector>
#include <glm/glm.hpp>
#include "_ModelLoaderMD2.h"
#include "_foodsystem.h"
#include "_player.h"

class _terrainsystem {
public:
    _terrainsystem();
    ~_terrainsystem();
    struct Terrain {
        int type; // 0 = bottle, 1 = crab
        glm::vec3 pos;
        glm::vec3 dir;
        float moveTimer;
        _ModelLoaderMD2* model;
    };

    std::vector<Terrain> terrain;

    void clear();
    void spawn(int level, float halfX, float halfZ, float floorY, glm::vec3 playerPos);
    void update(float dt, float halfX);
    void handleFoodCollision(std::vector<_food*>& foods);
    bool checkPlayerCollision(_player* player);

    void draw();
};

#endif
