// ================================================================
//  _foodsystem.cpp  –  Spawns and manages multiple food billboards
// ================================================================
#include "_foodsystem.h"
#include <cmath>
#include <iostream>

_foodsystem::_foodsystem() : foodCollectedThisFrame(0), megaHitsThisFrame(0) {}
_foodsystem::~_foodsystem() {}

void _foodsystem::init(_skyBox* skyRef) { sky = skyRef; }

// ================================================================
//  spawnFoods – mix of 5 food types: banana, cheese, donut, milk, melon
// ================================================================
void _foodsystem::spawnFoods(int numFoods, int level)
{
    struct FoodType {
        std::string texture;
        float       size;
        int         id;
    };

    std::vector<FoodType> foodTypes;

    switch(level)
    {
        // ========================================================
        // LEVEL 1 — Kitchen
        // ========================================================
        case 1:
            foodTypes = {
                { "images/cheese.png",  5.5f, 0 },
                { "images/milk.png",    6.5f, 1 },
                { "images/bread.png",   6.0f, 2 },
                { "images/banana.png",  5.0f, 3 }
            };
            break;

        // ========================================================
        // LEVEL 2 — Club
        // ========================================================
        case 2:
            foodTypes = {
                { "images/wine.png",    6.0f, 0 },
                { "images/cherry.png",  4.5f, 1 },
                { "images/lime.png",    4.5f, 2 },
                { "images/mojito.png",  6.0f, 3 }
            };
            break;

        // ========================================================
        // LEVEL 3 — Beach
        // ========================================================
        case 3:
            foodTypes = {
                { "images/wmelon.png",   6.5f, 0 },
                { "images/water.png",    5.5f, 1 },
                { "images/chips.png",    5.5f, 2 },
                { "images/icecream.png", 6.0f, 3 }
            };
            break;
    }


    float skyBottomY = sky->pos.y - sky->scale.y * 0.5f;
    float floorY     = skyBottomY + 1.0f;
    float halfX      = sky->scale.x * 0.45f;
    float halfZ      = sky->scale.z * 0.45f;

    // ── Mouse hole exclusion zone ────────────────────────────────
    // The hole is at approximately (-halfX_scene*0.60, -halfZ_scene+6).
    // _scene tells us where it is, but here we recompute from the
    // skybox geometry to keep this self-contained.  Food must spawn
    // OUTSIDE a generous safety zone (hole radius + magnet radius
    // + buffer) so the magnet doesn't suck a freshly-dropped food
    // into the hole before the player has even moved.
    float holeX = -(sky->scale.x * 0.5f) * 0.60f;
    float holeZ = -(sky->scale.z * 0.5f) + 6.0f;
    float holeRadius   = 6.0f;
    float magnetRadius = 18.0f;
    float exclusion    = holeRadius + magnetRadius + 6.0f;  // ~30 units

    for (int i = 0; i < numFoods; i++) {
        FoodType type = foodTypes[rand() % foodTypes.size()];

        _food* newFood = new _food(type.texture, type.size, type.id);

        // Reroll spawn position until it's clear of the hole zone
        // (capped at 30 attempts to avoid infinite loops on tiny maps).
        float fx, fz;
        for (int attempt = 0; attempt < 30; attempt++) {
            fx = ((rand()%200)/100.0f - 1.0f) * halfX;
            fz = ((rand()%200)/100.0f - 1.0f) * halfZ;
            float dx = fx - holeX, dz = fz - holeZ;
            if (dx*dx + dz*dz >= exclusion * exclusion) break;
        }
        newFood->physics.pos.x = fx;
        newFood->physics.pos.z = fz;
        newFood->physics.pos.y = floorY + 30.0f + (rand()%40);  // stagger drop
        foods.push_back(newFood);
    }
}

// ================================================================
//  handleCollisions – food vs food (sphere)
// ================================================================
void _foodsystem::handleCollisions()
{
    for (size_t i = 0; i < foods.size(); i++) {
        for (size_t j = i + 1; j < foods.size(); j++) {
            _food* a = foods[i];
            _food* b = foods[j];

            float dx = a->physics.pos.x - b->physics.pos.x;
            float dy = a->physics.pos.y - b->physics.pos.y;
            float dz = a->physics.pos.z - b->physics.pos.z;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz);

            float minDist = a->collisionRadius + b->collisionRadius;
            if (dist < minDist && dist > 0.0001f) {
                float nx = dx / dist, ny = dy / dist, nz = dz / dist;
                float overlap = minDist - dist;

                a->physics.pos.x += nx * overlap * 0.5f;
                a->physics.pos.y += ny * overlap * 0.5f;
                a->physics.pos.z += nz * overlap * 0.5f;
                b->physics.pos.x -= nx * overlap * 0.5f;
                b->physics.pos.y -= ny * overlap * 0.5f;
                b->physics.pos.z -= nz * overlap * 0.5f;

                float rvx = a->physics.velocity.x - b->physics.velocity.x;
                float rvy = a->physics.velocity.y - b->physics.velocity.y;
                float rvz = a->physics.velocity.z - b->physics.velocity.z;
                float velAlongNormal = rvx*nx + rvy*ny + rvz*nz;
                if (velAlongNormal > 0) continue;

                float impulse = -(1 + 0.6f) * velAlongNormal * 0.5f;
                a->physics.velocity.x += impulse * nx;
                a->physics.velocity.y += impulse * ny;
                a->physics.velocity.z += impulse * nz;
                b->physics.velocity.x -= impulse * nx;
                b->physics.velocity.y -= impulse * ny;
                b->physics.velocity.z -= impulse * nz;
            }
        }
    }
}

// ================================================================
// ================================================================
//  update – moves food, applies friction, keeps food away from walls
//
//  Food is NOT allowed to touch the walls.  Instead of collision /
//  bouncing (which can leave food stuck against an edge), we use a
//  soft inward force that pushes food away from any wall it gets
//  too close to.  The result: food always drifts back toward the
//  center of the room, never gets stuck, and the player can always
//  reach it from any side.
// ================================================================
void _foodsystem::update(float dt, float floorY)
{
    // Food must stay inside the actual room walls (skybox is 0.5 × scale
    // from origin to wall).  Using 0.50 here lets food roll all the way
    // to the back wall where the mouse hole sits — using 0.45 (as it was
    // before) created a 60-unit dead zone that food could never cross.
    float halfX  = sky->scale.x * 0.50f;
    float halfZ  = sky->scale.z * 0.50f;
    float buffer = 4.0f;     // visible margin so food never touches the wall

    for (auto& food : foods) {
        food->update(dt, floorY);

        float r = food->collisionRadius;

        // ── Keep the food's bounding sphere fully inside the room ──
        // The food's rightmost edge is pos.x + r, leftmost is pos.x - r,
        // and the room's inner walls are at ±(halfX - buffer).
        // Solve for pos.x:  pos.x + r ≤ halfX - buffer  →  pos.x ≤ halfX - buffer - r
        //                   pos.x - r ≥ -halfX + buffer →  pos.x ≥ -halfX + buffer + r
        float maxX = halfX - buffer - r;
        float minX = -halfX + buffer + r;
        float maxZ = halfZ - buffer - r;
        float minZ = -halfZ + buffer + r;

        // Hard clamp + kill any velocity heading further outward
        if (food->physics.pos.x > maxX) {
            food->physics.pos.x = maxX;
            if (food->physics.velocity.x > 0) food->physics.velocity.x = 0;
        }
        if (food->physics.pos.x < minX) {
            food->physics.pos.x = minX;
            if (food->physics.velocity.x < 0) food->physics.velocity.x = 0;
        }
        if (food->physics.pos.z > maxZ) {
            food->physics.pos.z = maxZ;
            if (food->physics.velocity.z > 0) food->physics.velocity.z = 0;
        }
        if (food->physics.pos.z < minZ) {
            food->physics.pos.z = minZ;
            if (food->physics.velocity.z < 0) food->physics.velocity.z = 0;
        }

        // Friction
        food->physics.velocity.x *= powf(0.99f, dt * 60.0f);
        food->physics.velocity.z *= powf(0.99f, dt * 60.0f);
    }
}

// ================================================================
//  checkFoodInHole – removes food when it enters the hole
//                    plus magnet effect that pulls food in
// ================================================================
void _foodsystem::checkFoodInHole(glm::vec3 holePos, float holeRadius,
                                   int& score, float dt)
{
    foodCollectedThisFrame = 0;

    // ── Magnet effect: pull food toward the hole ─────────────────
    // Subtle pull so food has to be actively pushed close to the hole
    // before it gets sucked in.  No more grabbing food across the room.
    float magnetRadius   = 60.0f;
    float magnetStrength = 200.0f;

    for (size_t i = 0; i < foods.size(); ) {
        _food* food = foods[i];
        float dx = food->physics.pos.x - holePos.x;
        float dz = food->physics.pos.z - holePos.z;
        float dist = sqrtf(dx*dx + dz*dz);

        if (dist < holeRadius) {
            foods.erase(foods.begin() + i);
            delete food;
            score += 10;          // 10 points per food
            foodCollectedThisFrame++;
            continue;
        }
        i++;

        if (dist < magnetRadius && dist > 0.001f) {
            float nx = -dx / dist, nz = -dz / dist;
            float t = 1.0f - (dist / magnetRadius);
            t = t * t;
            float strength = magnetStrength * t;
            food->physics.velocity.x += nx * strength * dt;
            food->physics.velocity.z += nz * strength * dt;
        }
    }
}

// ================================================================
//  handlePlayerCollisions – player pushes food, walls bounce food
// ================================================================
void _foodsystem::handlePlayerCollisions(_player* player,
                                          glm::vec3 playerMoveDir,
                                          bool playerIsDashing)
{
    megaHitsThisFrame = 0;

    // Note: walls deliberately do NOT block food.  Player movement is
    // clamped against walls in the scene; food can drift past the
    // visual wall boundary without getting stuck against it.  This
    // lets the player always reach food and push it back toward the
    // hole without the food locking up against an edge.
    float playerRadius = 2.0f;

    // Normal push strength vs. dashing-into-food MEGA-HIT strength.
    // Dashing sends the food flying about 5x harder so it can shoot
    // across the room toward the hole.
    float pushStrength = playerIsDashing ? 38.0f : 7.5f;

    for (auto& food : foods) {
        float foodR = food->collisionRadius;

        float dx = food->physics.pos.x - player->physics.pos.x;
        float dz = food->physics.pos.z - player->physics.pos.z;
        float dist = sqrtf(dx*dx + dz*dz);

        float minDist = foodR + playerRadius;
        if (dist < minDist && dist > 0.0001f) {
            float nx = dx / dist, nz = dz / dist;
            float overlap = minDist - dist;
            food->physics.pos.x += nx * overlap;
            food->physics.pos.z += nz * overlap;

            food->physics.velocity.x += playerMoveDir.x * pushStrength;
            food->physics.velocity.z += playerMoveDir.z * pushStrength;

            // MEGA-HIT: dash + collision = celebrated, scored, and animated
            if (playerIsDashing) {
                megaHitsThisFrame++;
            }
        }
    }
}
