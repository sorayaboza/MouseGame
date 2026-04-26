#ifndef _RENDERER_H
#define _RENDERER_H

#include "_ModelLoaderMD2.h"
#include <vector>
#include <glm/glm.hpp>

// -------- PLAYER RENDER DATA --------
struct PlayerRenderData {
    glm::vec3 pos;
    float rotY;

    _ModelLoaderMD2* model;
};

// -------- FOOD RENDER DATA --------
struct FoodRenderData {
    glm::vec3 pos;
    float rotY;

    _ModelLoaderMD2* model;
};

// -------- FRAME DATA --------
struct FrameRenderData {
    PlayerRenderData player;
    std::vector<FoodRenderData>* foods;
};

class _renderer {
public:
    _renderer();
    ~_renderer();

    void renderFrame(FrameRenderData& frame);

private:
    void drawPlayer(const PlayerRenderData& player);
    void drawFood(const FoodRenderData& food);
};

#endif
