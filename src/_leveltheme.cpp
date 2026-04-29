#include "_leveltheme.h"

_leveltheme::_leveltheme() {}
_leveltheme::~_leveltheme() {}

void _leveltheme::apply(int level,
                        _skyBox* sky,
                        _player* player,
                        _enemy* cat,
                        std::vector<_enemy*>& dogs)
{
    std::string base = "images/L" + std::to_string(level);

    // ---------- SKYBOX ----------
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
    std::string playerTex = "models/weretiger/Tygris" + std::to_string(level) + ".png";
    player->init("models/weretiger/tris.md2", playerTex);

    printf("Loading: %s\n", (base + "W1.png").c_str());

    // ---------- ENEMIES ----------
    cat->init(level, _enemy::TYPE_CAT);

    for (auto* d : dogs)
        d->init(level, _enemy::TYPE_DOG);
}
