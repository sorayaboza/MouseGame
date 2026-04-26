#ifndef _PLAYER_H
#define _PLAYER_H

#include <_ModelLoaderMD2.h>
#include <_physicsobject.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

// Represents the player (rat) in the game.
class _player {
public:
    _player();
    ~_player();

    void init(const std::string& modelPath);
    void update(float dt);

    _physicsobject physics;
    _ModelLoaderMD2* model;

    struct {
        float y = 0;
    } rot;
};

#endif
