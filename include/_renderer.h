#ifndef _RENDERER_H
#define _RENDERER_H

#include "_ModelLoaderMD2.h"
#include "_texloader.h"
#include <vector>
#include <glm/glm.hpp>

// Player – animated MD2 model (rat)
struct PlayerRenderData {
    glm::vec3        pos;
    float            rotY;
    _ModelLoaderMD2* model;
};

// Food – billboard textured quad
struct FoodRenderData {
    glm::vec3   pos;
    float       size;
    _texLoader* texture;
};

// Fart cloud
struct FartRenderData {
    glm::vec3 pos;
    float     life;
};

struct FrameRenderData {
    PlayerRenderData              player;
    std::vector<FoodRenderData>*  foods;
    std::vector<FartRenderData>*  farts;

    glm::vec3 mouseHolePos;
    float     mouseHoleRadius;
    glm::vec3 cameraPos;
};

class _renderer {
public:
    _renderer();
    ~_renderer();
    void renderFrame(FrameRenderData& frame);

private:
    void drawPlayer(const PlayerRenderData& p);
    void drawFoodBillboard(const FoodRenderData& food, const glm::vec3& camPos);
    void drawFart(const FartRenderData& fart);
    void drawMouseHole(const glm::vec3& pos, float radius);
};

#endif
