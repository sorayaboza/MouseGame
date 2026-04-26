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

// -------- FART RENDER DATA --------
struct FartRenderData {
    glm::vec3 pos;
};


// -------- FRAME DATA --------
struct FrameRenderData {
    PlayerRenderData player;
    std::vector<FoodRenderData>* foods;

    std::vector<FartRenderData>* farts;

    glm::vec3 mouseHolePos;
};


class _renderer {
public:
    _renderer();
    ~_renderer();

    void renderFrame(FrameRenderData& frame);

private:
    void drawPlayer(const PlayerRenderData& player);
    void drawFood(const FoodRenderData& food);
    void drawFart(const FartRenderData& fart);
    void drawMouseHole(const glm::vec3& pos);
};

#endif
