// ================================================================
//  _enemy.cpp – Cat/Dog enemy with full animation set
//
//  Animations:
//    PATROL     → RUN  (frames 40-45)  default while moving around
//    CHASE      → JUMP (frames 66-71)  fast pursuit
//    DISTRACTED → PAIN (frames 54-65)  recoiling from fart
// ================================================================
#include "_enemy.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float randFloat(float lo, float hi) {
    return lo + (hi - lo) * ((float)rand() / RAND_MAX);
}

_enemy::_enemy()
    : state(PATROL), type(TYPE_CAT), facingAngle(0.0f),
      patrolSpeed(18.0f), chaseSpeed(40.0f),
      fovHalfAngle(0.87f), fovRange(90.0f),
      catchRadius(7.0f), distractRadius(30.0f),
      dirTimer(0.0f), lostTimer(0.0f), distractTimer(0.0f),
      modelScale(0.45f)
{
    pos     = {0, 0, 0};
    facing  = {0, 0, -1};
    model   = new _ModelLoaderMD2();
}
_enemy::~_enemy() {}

void _enemy::init(int level, Type t)
{
    type = t;
    if (type == TYPE_CAT) {
        model->initModel("models/Nightcrawler/tris.md2",
                         (char*)"models/Nightcrawler/NightC.jpg");
        modelScale     = 0.40f;
        patrolSpeed    = 16.0f + level * 5.0f;
        chaseSpeed     = 36.0f + level * 12.0f;
        fovRange       = 80.0f + level * 12.0f;
        fovHalfAngle   = 0.80f + level * 0.06f;
        catchRadius    = 8.0f;
    } else {
        model->initModel("models/awolf/tris.md2",
                         (char*)"models/awolf/wolfskin.jpg");
        modelScale     = 0.35f;
        patrolSpeed    = 12.0f + level * 3.0f;
        chaseSpeed     = 28.0f + level * 8.0f;
        fovRange       = 60.0f + level * 8.0f;
        fovHalfAngle   = 0.65f + level * 0.04f;
        catchRadius    = 6.0f;
    }

    pos = { 80.0f, 0.0f, 80.0f };
    dirTimer = randFloat(2.0f, 4.0f);
    pickNewDirection(200.0f, 200.0f);
}

bool _enemy::update(float dt,
                    const glm::vec3& playerPos,
                    const std::vector<Fart>& farts,
                    float halfX, float halfZ)
{
    // ── Distraction (highest priority) ───────────────────────────
    if (isFartNearby(farts)) {
        state = DISTRACTED;
        distractTimer = 3.5f;
        glm::vec3 fartPos = nearestFartPos(farts);
        float dx = fartPos.x - pos.x;
        float dz = fartPos.z - pos.z;
        if (dx*dx + dz*dz > 0.001f) facingAngle = atan2f(dx, dz);
        updateFacing();
        // PAIN animation while distracted
        model->actionTrigger = model->PAIN;
        model->actions();
        return false;
    }

    if (state == DISTRACTED) {
        distractTimer -= dt;
        if (distractTimer <= 0.0f) {
            state = PATROL;
            dirTimer = randFloat(2.0f, 4.0f);
        }
        // Stay in PAIN animation while still recovering
        model->actionTrigger = model->PAIN;
        model->actions();
        return false;
    }

    bool sees = isPlayerInLOS(playerPos);

    // ── CHASE state – JUMP animation ─────────────────────────────
    if (state == CHASE) {
        if (!sees) {
            lostTimer += dt;
            if (lostTimer >= 2.0f) {
                state = PATROL;
                lostTimer = 0;
                dirTimer = randFloat(2.0f, 4.0f);
            }
        } else lostTimer = 0;

        float dx = playerPos.x - pos.x;
        float dz = playerPos.z - pos.z;
        float dist = sqrtf(dx*dx + dz*dz);
        if (dist < catchRadius) return true;
        if (dist > 0.001f) {
            facingAngle = atan2f(dx, dz);
            updateFacing();
            pos.x += facing.x * chaseSpeed * dt;
            pos.z += facing.z * chaseSpeed * dt;
        }
        model->actionTrigger = model->JUMP;
        model->actions();
    }
    // ── PATROL state – RUN animation ─────────────────────────────
    else {
        if (sees) { state = CHASE; lostTimer = 0; }
        else {
            // Predict next position
            float nextX = pos.x + facing.x * patrolSpeed * dt;
            float nextZ = pos.z + facing.z * patrolSpeed * dt;

            // ── Smarter wall avoidance to prevent corner glitches ──
            //   Instead of clamping after the fact, REJECT the move
            //   if it would put us against a wall and pick a new
            //   direction pointing away from that wall.
            bool blocked = false;
            float margin = 10.0f;

            if (nextX < -halfX + margin) {
                // Heading into left wall – turn to face right side
                facingAngle = randFloat(0.2f * (float)M_PI, 0.8f * (float)M_PI);
                blocked = true;
            } else if (nextX > halfX - margin) {
                // Heading into right wall – face left
                facingAngle = randFloat(-0.8f * (float)M_PI, -0.2f * (float)M_PI);
                blocked = true;
            }
            if (nextZ < -halfZ + margin) {
                // Heading into back wall – face forward
                facingAngle = randFloat(-0.4f * (float)M_PI, 0.4f * (float)M_PI);
                blocked = true;
            } else if (nextZ > halfZ - margin) {
                // Heading into front wall – face back
                facingAngle = randFloat((float)M_PI - 0.4f * (float)M_PI,
                                        (float)M_PI + 0.4f * (float)M_PI);
                blocked = true;
            }

            if (blocked) {
                updateFacing();
                dirTimer = randFloat(1.5f, 3.0f);
                // Don't move this frame – next frame we'll go in the
                // new direction.  This prevents corner deadlocks.
            } else {
                pos.x = nextX;
                pos.z = nextZ;

                dirTimer -= dt;
                if (dirTimer <= 0.0f) {
                    pickNewDirection(halfX, halfZ);
                    dirTimer = randFloat(2.5f, 5.0f);
                }
            }

            // Hard clamp as a final safety net
            if (pos.x < -halfX + margin) pos.x = -halfX + margin;
            if (pos.x >  halfX - margin) pos.x =  halfX - margin;
            if (pos.z < -halfZ + margin) pos.z = -halfZ + margin;
            if (pos.z >  halfZ - margin) pos.z =  halfZ - margin;

            // RUN animation while patrolling
            model->actionTrigger = model->RUN;
            model->actions();
        }
    }
    return false;
}

// ================================================================
//  draw – MD2 model with proper facing direction
//
//  Order of rotations is critical:
//    1. -90° about world X first (lays MD2 model upright)
//    2. then yaw about world Y so its forward (+X) points along
//       (sin(facingAngle), 0, cos(facingAngle))
//
//  The MD2 model's "forward" is +X.  After R_X(-90):
//    a vertex at (1,0,0) stays at (1,0,0) (still facing +X).
//  We need to rotate around Y so that +X points to facing direction.
//    glRotatef(theta, 0,1,0) sends (1,0,0) to (cos(theta), 0, -sin(theta))
//    Want this = (sin(angle), 0, cos(angle))
//    → theta = 90° - degrees(angle)
// ================================================================
void _enemy::draw()
{
    float halfModelHeight = 22.0f * modelScale;

    glPushMatrix();
        glTranslatef(pos.x, pos.y + halfModelHeight, pos.z);
        // After R_X(-90), the MD2 model is upright with +X still pointing
        // forward.  R_Y(angle - 90) then maps +X to (sin(angle), 0, cos(angle))
        // which is the desired facing direction (sin/cos of facingAngle).
        glRotatef(glm::degrees(facingAngle) - 90.0f, 0, 1, 0);
        glRotatef(-90, 1, 0, 0);
        glScalef(modelScale, modelScale, modelScale);
        model->Draw();
    glPopMatrix();
}

void _enemy::drawLOS()
{
    float baseY = pos.y + 0.3f;
    int   segs  = 40;

    float r,g,b,alpha;
    if      (state == PATROL)     { r=0.20f; g=0.90f; b=0.20f; alpha=0.20f; }
    else if (state == CHASE)      { r=1.00f; g=0.10f; b=0.10f; alpha=0.38f; }
    else                          { r=1.00f; g=0.85f; b=0.00f; alpha=0.26f; }

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glColor4f(r,g,b,alpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(pos.x, baseY, pos.z);
    for (int i = 0; i <= segs; i++) {
        float t = (float)i / segs;
        float a = facingAngle - fovHalfAngle + 2.0f * fovHalfAngle * t;
        glVertex3f(pos.x + fovRange*sinf(a), baseY, pos.z + fovRange*cosf(a));
    }
    glEnd();

    glColor4f(r,g,b,std::min(alpha*3.0f,1.0f));
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    glVertex3f(pos.x, baseY, pos.z);
    for (int i = 0; i <= segs; i++) {
        float t = (float)i / segs;
        float a = facingAngle - fovHalfAngle + 2.0f * fovHalfAngle * t;
        glVertex3f(pos.x + fovRange*sinf(a), baseY, pos.z + fovRange*cosf(a));
    }
    glVertex3f(pos.x, baseY, pos.z);
    glEnd();
    glLineWidth(1.0f);

    glDepthMask(GL_TRUE);
    glPopAttrib();
}

bool _enemy::isPlayerInLOS(const glm::vec3& playerPos) const
{
    float dx = playerPos.x - pos.x;
    float dz = playerPos.z - pos.z;
    float dist = sqrtf(dx*dx + dz*dz);
    if (dist > fovRange) return false;
    if (dist < 0.001f)   return true;
    float toAngle = atan2f(dx, dz);
    float diff = toAngle - facingAngle;
    while (diff >  (float)M_PI) diff -= 2.0f*(float)M_PI;
    while (diff < -(float)M_PI) diff += 2.0f*(float)M_PI;
    return fabsf(diff) <= fovHalfAngle;
}
bool _enemy::isFartNearby(const std::vector<Fart>& farts) const
{
    for (const auto& f : farts) {
        float dx = f.pos.x - pos.x, dz = f.pos.z - pos.z;
        if (dx*dx + dz*dz <= distractRadius * distractRadius) return true;
    }
    return false;
}
glm::vec3 _enemy::nearestFartPos(const std::vector<Fart>& farts) const
{
    glm::vec3 best = farts[0].pos;
    float bestDist = 1e9f;
    for (const auto& f : farts) {
        float dx = f.pos.x - pos.x, dz = f.pos.z - pos.z;
        float d = dx*dx + dz*dz;
        if (d < bestDist) { bestDist = d; best = f.pos; }
    }
    return best;
}
void _enemy::pickNewDirection(float halfX, float halfZ)
{
    facingAngle = randFloat(0.0f, 2.0f*(float)M_PI);
    updateFacing();
}
void _enemy::updateFacing()
{
    facing.x = sinf(facingAngle);
    facing.y = 0.0f;
    facing.z = cosf(facingAngle);
}
