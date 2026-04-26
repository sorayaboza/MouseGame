#include "_foodsystem.h"
#include <cmath>
#include <iostream>

_foodsystem::_foodsystem() { /* ctor */ }
_foodsystem::~_foodsystem() { /* dtor */ }

void _foodsystem::init(_skyBox* skyRef) {
    sky = skyRef;
}

// Create several food objects that fall and bounce inside the sky box.
void _foodsystem::spawnFoods(int numFoods) {
    // List of possible food models
    struct FoodType {
        std::string model;
        std::string texture;
        float scale;
    };

    std::vector<FoodType> foodTypes = {
        {"models/milk/tris.md2", "images/milk.png", 0.8f},
        {"models/milk/tris.md2",  "images/milk.png",  1.2f}
    };

    // Determine playable boundaries from sky box size
    float skyBottomY = sky->pos.y - sky->scale.y * 0.5f;
    float floorY = skyBottomY + 1.0f; // 1 unit above bottom

    float halfX = sky->scale.x * 0.5f;
    float halfZ = sky->scale.z * 0.5f;

    for(int i=0; i<numFoods; i++) {
        FoodType type = foodTypes[rand() % foodTypes.size()];
        _food* newFood = new _food(type.model, type.scale);

        newFood->physics.pos.x = ((rand()%100)/100.0f)*(2*halfX)-halfX; // Random X position inside sky box bounds
        newFood->physics.pos.z = ((rand()%100)/100.0f)*(2*halfZ)-halfZ; // Random Z position inside sky box bounds

        float spawnHeight = floorY + 40.0f;  // spawn 20 units above the floor
        newFood->physics.pos.y = spawnHeight;

        foods.push_back(newFood); // Store food in vector so it can be updated/drawn later
    }

}

/* handleCollisions()

    Detects collisions between food objects using sphere collision.

    Steps:
    1. Compute distance between food centers.
    2. If distance < combined radius, collision occurred.
    3. Push the objects apart along the collision normal.
    4. Apply a collision impulse so momentum transfers between foods.
       This causes both objects to react naturally when they collide.
*/
void _foodsystem::handleCollisions() {

    float radius = 2.0f; // collision radius for each food

    // Compare every pair of foods
    for (size_t i = 0; i < foods.size(); i++) {
        for (size_t j = i + 1; j < foods.size(); j++) {

            _food* a = foods[i];
            _food* b = foods[j];

            // Vector from food B to food A
            float dx = a->physics.pos.x - b->physics.pos.x;
            float dy = a->physics.pos.y - b->physics.pos.y;
            float dz = a->physics.pos.z - b->physics.pos.z;

            // Distance between food centers
            float dist = sqrt(dx*dx + dy*dy + dz*dz);

            // Minimum distance before collision occurs
            float minDist = radius * 2.0f;

            // Check if foods are intersecting
            if (dist < minDist && dist > 0.0001f) {

                // Normalize the collision direction (collision normal)
                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;

                // Amount the objects overlap
                float overlap = minDist - dist;

                // Push objects apart equally so they no longer intersect
                a->physics.pos.x += nx * overlap * 0.5f;
                a->physics.pos.y += ny * overlap * 0.5f;
                a->physics.pos.z += nz * overlap * 0.5f;

                b->physics.pos.x -= nx * overlap * 0.5f;
                b->physics.pos.y -= ny * overlap * 0.5f;
                b->physics.pos.z -= nz * overlap * 0.5f;

                // --- Collision Physics ---

                // Relative velocity between the two foods
                float rvx = a->physics.velocity.x - b->physics.velocity.x;
                float rvy = a->physics.velocity.y - b->physics.velocity.y;
                float rvz = a->physics.velocity.z - b->physics.velocity.z;

                // Velocity along the collision normal
                float velAlongNormal = rvx*nx + rvy*ny + rvz*nz;

                // If objects are moving away from each other, skip resolution
                if (velAlongNormal > 0)
                    continue;

                // Coefficient of restitution (bounciness)
                float bounce = 1.0f;

                // Calculate collision impulse
                float impulse = -(1 + bounce) * velAlongNormal;

                // Divide impulse for equal-mass objects
                impulse *= 0.5f;

                // Apply impulse to both foods (equal and opposite forces)
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

void _foodsystem::update(float dt, float floorY) {
    for (auto& food : foods) {
        food->update(dt, floorY);
        food->physics.velocity.x *= pow(0.99f, dt * 60.0f);
        food->physics.velocity.z *= pow(0.99f, dt * 60.0f);
    }
}

// Detects collision with food and mouse hole.
void _foodsystem::checkFoodInHole(glm::vec3 holePos, float holeRadius, int& score, float dt) {
    float magnetRadius = 30.0f;   // how far the magnet reaches
    float magnetStrength = 70.0f; // how strong the pull is

    for (size_t i = 0; i < foods.size(); /* no i++ here */) {
        _food* food = foods[i];

        // Compute vector from hole center to food
        float dx = food->physics.pos.x - holePos.x;
        //float dy = food->physics.pos.y - mouseHolePos.y;
        float dz = food->physics.pos.z - holePos.z;

        float dist = sqrt(dx*dx + dz*dz);
        // float dist = sqrt(dx*dx + dy*dy + dz*dz); // with y version

        // If food is inside hole
        if (dist < holeRadius) {
            // Food has entered the hole!
            foods.erase(foods.begin() + i); // 1. Remove from the foods vector
            delete food; // 2. Delete the food object
            score += 1; // 3. Increment score or trigger event
            cout << "SCORE: " << score << endl;

            // Do NOT increment i since we erased an element
            continue;
        }

        i++; // Only increment if food wasn't removed

        if (dist < magnetRadius && dist > 0.001f) {
            float nx = -dx / dist; // direction TOWARD hole
            float nz = -dz / dist;

            // stronger when closer (feels nice)
            float strength = magnetStrength * (1.0f - (dist / magnetRadius));

            // apply pull
            food->physics.velocity.x += nx * strength * dt;
            food->physics.velocity.z += nz * strength * dt;
        }
    }
}

// Detects collisions between foods and the player.
void _foodsystem::handlePlayerCollisions(_player* player, glm::vec3 playerMoveDir) {
    float halfX = sky->scale.x * 0.50f;  // skybox X boundary
    float halfZ = sky->scale.z * 0.50f;  // skybox Z boundary
    float foodRadius = 2.0f;   // size of the food collision cube/sphere
    float radius = 2.0f; // collision radius

    for (auto& food : foods) {

        // Vector from player to food
        float dx = food->physics.pos.x - player->physics.pos.x;
        float dy = 0.0f; // Keeps food ground-based
        float dz = food->physics.pos.z - player->physics.pos.z;

        float dist = sqrt(dx*dx + dy*dy + dz*dz);
        float minDist = radius * 2.0f;

        if (dist < minDist && dist > 0.0001f) {
            // Normalize push direction
            float nx = dx / dist;
            float ny = dy / dist;
            float nz = dz / dist;

            // Separate the objects
            float overlap = minDist - dist;

            food->physics.pos.x += nx * overlap;
            food->physics.pos.y += ny * overlap;
            food->physics.pos.z += nz * overlap;

            // Apply push velocity from the player
            float pushStrength = 6.0f;

            food->physics.velocity.x += playerMoveDir.x * pushStrength;
            food->physics.velocity.z += playerMoveDir.z * pushStrength;
        }

        // Sky box boundary: Keeps food inside the world
        if (food->physics.pos.x - foodRadius < -halfX) {
            food->physics.pos.x = -halfX + foodRadius;
            food->physics.velocity.x *= -0.9f;
        }
        if (food->physics.pos.x + foodRadius > halfX) {
            food->physics.pos.x = halfX - foodRadius;
            food->physics.velocity.x *= -0.9f;
        }
        if (food->physics.pos.z - foodRadius < -halfZ) {
            food->physics.pos.z = -halfZ + foodRadius;
            food->physics.velocity.z *= -0.9f;
        }
        if (food->physics.pos.z + foodRadius > halfZ) {
            food->physics.pos.z = halfZ - foodRadius;
            food->physics.velocity.z *= -0.9f;
        }

        float cornerRadius = 6.0f;

        glm::vec3 corners[4] = {
            { -halfX, 0, -halfZ },
            {  halfX, 0, -halfZ },
            { -halfX, 0,  halfZ },
            {  halfX, 0,  halfZ }
        };

        for (int i = 0; i < 4; i++) {
            float dx = food->physics.pos.x - corners[i].x;
            float dz = food->physics.pos.z - corners[i].z;

            float dist = sqrt(dx*dx + dz*dz);

            if (dist < cornerRadius && dist > 0.0001f) {
                float nx = dx / dist;
                float nz = dz / dist;

                float overlap = cornerRadius - dist;

                food->physics.pos.x += nx * overlap;
                food->physics.pos.z += nz * overlap;

                float bounce = 0.6f;
                float dot = food->physics.velocity.x * nx + food->physics.velocity.z * nz;

                if (dot < 0) {
                    food->physics.velocity.x -= (1 + bounce) * dot * nx;
                    food->physics.velocity.z -= (1 + bounce) * dot * nz;
                }
            }
        }
    }
}
