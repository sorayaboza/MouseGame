#ifndef _PLAYER_H
#define _PLAYER_H

#include <_ModelLoaderMD2.h>
#include <_texloader.h>
#include <_physicsobject.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

// The player (rat). Uses an animated MD2 model with proper texture.
class _player {
public:
    _player();
    ~_player();

    void init(const std::string& modelPath, const std::string& texturePath);
    void update(float dt);

    _physicsobject   physics;
    _ModelLoaderMD2* model;       // animated MD2 model
    bool             isMoving;
    float            animTime;
    struct { float y = 0; } rot;
};

#endif
