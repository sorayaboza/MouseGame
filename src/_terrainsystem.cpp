#include "_terrainsystem.h"
#include <cmath>
#include <cstdlib>

_terrainsystem::_terrainsystem() { /*ctor*/}
_terrainsystem::~_terrainsystem() { /*dtor*/ }

void _terrainsystem::clear() {
    for (auto& t : terrain) delete t.model;
    terrain.clear();
}

void _terrainsystem::spawn(int level, float halfX, float halfZ, float floorY, glm::vec3 playerPos)
{
    float minDist = 20.0f;

    if (level == 2) {
        for (int i = 0; i < 10; i++) {
            Terrain t;
            t.type = 0;
            t.model = new _ModelLoaderMD2();
            t.model->initModel("models/Bottle/tris.md2",
                               (char*)"models/Bottle/Bottle.png");

            // ── Pin the bottle to a single frame so it doesn't animate.
            //    The MD2 Draw() method advances between StartFrame and
            //    EndFrame each call; setting them both to 0 leaves it
            //    perfectly still.
            t.model->StartFrame = 0;
            t.model->EndFrame   = 0;

            for (int attempt = 0; attempt < 20; attempt++) {
                float x = (rand() % (int)(halfX * 1.8f)) - halfX * 0.9f;
                float z = (rand() % (int)(halfZ * 1.8f)) - halfZ * 0.9f;

                float dx = x - playerPos.x;
                float dz = z - playerPos.z;

                if (dx*dx + dz*dz > minDist * minDist) {
                    t.pos = { x, floorY, z };
                    break;
                }
            }
            terrain.push_back(t);
        }
    }

    if (level == 3) {
        // Track previously-placed lobster positions so we can space
        // them apart properly.  Also gives us scoped data for
        // computing reasonable Z distribution.
        const float LOBSTER_MIN_SEPARATION = 35.0f;   // min distance between lobsters
        const float PLAYER_MIN_SEPARATION  = 30.0f;   // min distance from player
        std::vector<glm::vec3> placed;

        for (int i = 0; i < 5; i++) {
            Terrain t;
            t.type = 1;
            t.model = new _ModelLoaderMD2();
            t.model->initModel("models/Lobster/tris.md2",
                               (char*)"models/Lobster/Lobster.png");

            // Pin to a single frame – no flapping/twitching animation.
            // (This also makes the orientation predictable; otherwise
            // mid-animation frames can make the model appear to face
            // a different direction at random moments.)
            t.model->StartFrame = 0;
            t.model->EndFrame   = 0;

            // Try up to 60 random positions to find one that's far
            // enough from the player AND from every other already-
            // placed lobster.  60 attempts is plenty for 5 lobsters
            // in a 300x300 room with 35-unit separation.
            bool placedOk = false;
            for (int attempt = 0; attempt < 60 && !placedOk; attempt++) {
                float x = ((float)rand() / RAND_MAX) * (halfX * 1.7f) - halfX * 0.85f;
                float z = ((float)rand() / RAND_MAX) * (halfZ * 1.5f) - halfZ * 0.75f;

                float dxP = x - playerPos.x;
                float dzP = z - playerPos.z;
                if (dxP*dxP + dzP*dzP < PLAYER_MIN_SEPARATION * PLAYER_MIN_SEPARATION)
                    continue;

                // Check separation from every previously-placed lobster
                bool tooClose = false;
                for (const auto& p : placed) {
                    float ddx = x - p.x;
                    float ddz = z - p.z;
                    if (ddx*ddx + ddz*ddz < LOBSTER_MIN_SEPARATION * LOBSTER_MIN_SEPARATION) {
                        tooClose = true;
                        break;
                    }
                }
                if (tooClose) continue;

                t.pos = { x, floorY, z };
                placed.push_back(t.pos);
                placedOk = true;
            }

            // Last-ditch fallback if 60 attempts couldn't find a spot
            // (shouldn't happen with these distances but just in case)
            if (!placedOk) {
                float angle = (float)i * 1.2566f;        // 2π/5 = 1.2566
                float r     = halfX * 0.55f;
                t.pos = { cosf(angle) * r, floorY, sinf(angle) * r };
                placed.push_back(t.pos);
            }

            // Stagger: alternate starting direction and offset the
            // timer per-lobster so they don't all reverse at once.
            t.dir = { (i % 2 == 0) ? 1.0f : -1.0f, 0, 0 };
            t.moveTimer = 6.0f - (i * 0.8f);   // 6.0, 5.2, 4.4, 3.6, 2.8
            terrain.push_back(t);
        }
    }
}

void _terrainsystem::update(float dt, float halfX) {
    const float LOBSTER_SPEED  = 60.0f;   // faster (was 20)
    const float LOBSTER_RANGE  = 6.0f;    // seconds before voluntary reverse,
                                          // ~ 6 * 28 = 168 units of travel
                                          // (was 2s * 20 = 40 units, way too short)
    const float WALL_PADDING   = 14.0f;   // stay this far from a side wall
                                          // (was 10 – wider margin so the
                                          // lobster doesn't visually clip the
                                          // wall before it bounces)
    const float LOBSTER_BODY_R = 8.0f;    // approx. body radius for lobster–
                                          // lobster collision response

    for (auto& t : terrain) {
        if (t.type == 1) {
            t.pos.x += t.dir.x * LOBSTER_SPEED * dt;

            // Voluntary direction change every LOBSTER_RANGE seconds.
            t.moveTimer -= dt;
            if (t.moveTimer <= 0.0f) {
                t.dir.x *= -1.0f;
                t.moveTimer = LOBSTER_RANGE;
            }

            // Wall bounce – clamp position inside the room AND force
            // direction inward.  Just flipping dir.x isn't enough: a
            // lobster that's already past the boundary will keep
            // flipping every frame and remain stuck on the wall.
            if (t.pos.x < -halfX + WALL_PADDING) {
                t.pos.x = -halfX + WALL_PADDING;   // clamp inside
                if (t.dir.x < 0) t.dir.x = +1.0f;  // force outward
                t.moveTimer = LOBSTER_RANGE;
            }
            else if (t.pos.x > halfX - WALL_PADDING) {
                t.pos.x = halfX - WALL_PADDING;
                if (t.dir.x > 0) t.dir.x = -1.0f;
                t.moveTimer = LOBSTER_RANGE;
            }
        }
    }

    // ── Lobster-vs-lobster separation ────────────────────────────
    // Without this, two lobsters on adjacent X paths can drift into
    // each other and visually fuse.  Using a simple O(n²) pairwise
    // check (n=5 so this is trivial) we push overlapping lobsters
    // apart along the line between them and bounce their X
    // direction so they don't keep grinding into each other.
    const float MIN_SEP   = LOBSTER_BODY_R * 2.0f;  // 16 units centre-to-centre
    const float MIN_SEP_SQ = MIN_SEP * MIN_SEP;

    for (size_t i = 0; i < terrain.size(); i++) {
        if (terrain[i].type != 1) continue;
        for (size_t j = i + 1; j < terrain.size(); j++) {
            if (terrain[j].type != 1) continue;

            float dx = terrain[i].pos.x - terrain[j].pos.x;
            float dz = terrain[i].pos.z - terrain[j].pos.z;
            float d2 = dx*dx + dz*dz;
            if (d2 >= MIN_SEP_SQ || d2 < 0.0001f) continue;   // not overlapping

            float dist = sqrtf(d2);
            float overlap = MIN_SEP - dist;
            float nx = dx / dist;
            float nz = dz / dist;

            // Push each one half the overlap apart.
            terrain[i].pos.x += nx * overlap * 0.5f;
            terrain[i].pos.z += nz * overlap * 0.5f;
            terrain[j].pos.x -= nx * overlap * 0.5f;
            terrain[j].pos.z -= nz * overlap * 0.5f;

            // Bounce X directions so they move away from each other
            // for the foreseeable future (avoids re-colliding on the
            // next frame after the push).
            if (terrain[i].pos.x < terrain[j].pos.x) {
                terrain[i].dir.x = -fabsf(terrain[i].dir.x);
                terrain[j].dir.x =  fabsf(terrain[j].dir.x);
            } else {
                terrain[i].dir.x =  fabsf(terrain[i].dir.x);
                terrain[j].dir.x = -fabsf(terrain[j].dir.x);
            }
            terrain[i].moveTimer = LOBSTER_RANGE;
            terrain[j].moveTimer = LOBSTER_RANGE;
        }
    }
}

void _terrainsystem::handleFoodCollision(std::vector<_food*>& foods) {
    for (auto* f : foods) {
        for (auto& t : terrain) {
            float dx = f->physics.pos.x - t.pos.x;
            float dz = f->physics.pos.z - t.pos.z;
            float dist = sqrt(dx*dx + dz*dz);

            if (dist < 6.0f && dist > 0.001f) {
                float push = 0.5f;
                f->physics.pos.x += (dx / dist) * push;
                f->physics.pos.z += (dz / dist) * push;
            }
        }
    }
}

bool _terrainsystem::checkPlayerCollision(_player* player) {
    for (auto& t : terrain) {
        float dx = player->physics.pos.x - t.pos.x;
        float dz = player->physics.pos.z - t.pos.z;
        float dist = sqrt(dx*dx + dz*dz);

        if (dist < 6.0f) {
            return true;
        }
    }
    return false;
}

void _terrainsystem::draw() {
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1,1,1);

    for (auto& t : terrain) {
        glPushMatrix();

        float scale = (t.type == 0) ? 0.25f : 0.35f;
        float yOffset = (t.type == 0) ? -2.0f : 5.0f;

        glTranslatef(t.pos.x, t.pos.y + yOffset, t.pos.z);

        if (t.type == 0) {
            // Bottle – stand it upright, no facing-direction rotation
            glRotatef(-90, 1, 0, 0);
        } else {
            // Lobster – face the FRONT wall (+Z), which is the
            // wall opposite the sand castle and where the player
            // spawns.  In MD2/Quake convention the model's forward
            // axis is +X.  After the X-axis stand-up (-90° around
            // world X), +X is still the forward in world space.
            // We then yaw -90° around world Y to rotate forward
            // from +X to +Z.
            //
            // OpenGL applies the LAST glRotatef call FIRST to the
            // vertex, so the yaw goes BEFORE the standup in source.
            glRotatef(-90, 0, 1, 0);    // 2nd applied: yaw +X → +Z
            glRotatef(-90, 1, 0, 0);    // 1st applied: stand up
        }

        glScalef(scale, scale, scale);

        if (t.model) t.model->Draw();

        glPopMatrix();
    }
}
