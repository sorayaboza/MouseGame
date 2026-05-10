#include "_leveltheme.h"
#include <cstdio>
#include <string>

_leveltheme::_leveltheme() {}
_leveltheme::~_leveltheme() {}

void _leveltheme::apply(int level,
                        _skyBox* sky,
                        _player* player,
                        _enemy* cat,
                        std::vector<_enemy*>& dogs)
{
    // First-time-init flag: we MUST do a full initModel() the first
    // time each model is touched (to parse the MD2 binary).  But on
    // subsequent levels we only swap the texture – much faster.
    static bool firstApply = true;

    std::string base = "images/L" + std::to_string(level);

    // ---------- SKYBOX ----------
    // Skybox textures change every level, so they're always re-loaded.
    char fileName[64];

    sprintf(fileName, "images/L%dW1.png", level); // Wall 1
    sky->myTex[0].loadTexture(fileName);

    sprintf(fileName, "images/L%dWH.png", level); // Wall Hole
    sky->myTex[1].loadTexture(fileName);

    sprintf(fileName, "images/L%dWF.png", level); // Ceiling
    sky->myTex[2].loadTexture(fileName);

    sprintf(fileName, "images/L%dWF.png", level); // Floor
    sky->myTex[3].loadTexture(fileName);

    sprintf(fileName, "images/L%dW2.png", level); // Wall 2
    sky->myTex[4].loadTexture(fileName);

    sprintf(fileName, "images/L%dW1.png", level); // Wall 3
    sky->myTex[5].loadTexture(fileName);

    // ---------- PLAYER ----------
    std::string playerTex = "models/Rat/Rat" + std::to_string(level) + ".png";
    if (firstApply) {
        player->init("models/Rat/tris.md2", playerTex);
    } else {
        // Just swap the texture – geometry already loaded.
        if (player->model)
            player->model->setTexture(playerTex.c_str());
    }

    printf("Loading: %s\n", (base + "W1.png").c_str());

    // ---------- ENEMIES ----------
    cat->init(level, _enemy::TYPE_CAT);

    std::string catTex = "models/Cat/Cat" + std::to_string(level) + ".png";
    if (firstApply) {
        cat->model->initModel("models/Cat/tris.md2", (char*)catTex.c_str());
    } else {
        cat->model->setTexture(catTex.c_str());
    }

    for (auto* d : dogs)
    {
        d->init(level, _enemy::TYPE_DOG);

        // Dogs are freshly allocated in startLevel each level (the
        // dog count changes), so we always need a full initModel.
        std::string dogTex = "models/Dog/Dog" + std::to_string(level) + ".png";
        d->model->initModel("models/Dog/tris.md2", (char*)dogTex.c_str());
    }

    firstApply = false;
}
