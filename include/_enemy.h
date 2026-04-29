#ifndef _ENEMY_H
#define _ENEMY_H

#include <_common.h>
#include <_ModelLoaderMD2.h>
#include <_abilities.h>
#include <glm/glm.hpp>
#include <vector>

// ================================================================
//  _enemy  –  Enemy AI with MD2 model
//
//  Two enemy types:
//    TYPE_CAT  –  main enemy, larger and faster (Nightcrawler MD2)
//    TYPE_DOG  –  secondary enemies, smaller and slower (awolf MD2)
// ================================================================
class _enemy {
public:
    enum State { PATROL, CHASE, DISTRACTED };
    enum Type  { TYPE_CAT, TYPE_DOG };

    State state;
    Type  type;

    glm::vec3 pos;
    float     facingAngle;
    glm::vec3 facing;

    float patrolSpeed;
    float chaseSpeed;
    float fovHalfAngle;
    float fovRange;
    float catchRadius;
    float distractRadius;

    float dirTimer;
    float lostTimer;
    float distractTimer;

    _ModelLoaderMD2* model;
    float            modelScale;

    _enemy();
    ~_enemy();

    // type=TYPE_CAT for main, TYPE_DOG for sidekicks
    void init(int level, Type t);
    bool update(float dt,
                const glm::vec3& playerPos,
                const std::vector<Fart>& farts,
                float halfX, float halfZ);
    void draw();
    void drawLOS();

private:
    bool isPlayerInLOS(const glm::vec3& playerPos) const;
    bool isFartNearby(const std::vector<Fart>& farts) const;
    glm::vec3 nearestFartPos(const std::vector<Fart>& farts) const;
    void pickNewDirection(float halfX, float halfZ);
    void updateFacing();
};

#endif
