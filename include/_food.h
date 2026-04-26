#ifndef _FOOD_H
#define _FOOD_H

#include <string>
#include <_ModelLoaderMD2.h>
#include <_physicsobject.h>
#include <_texloader.h>

// Represents a food item in the game.
class _food {
public:
    _food(const std::string& modelPath, float scale);
    ~_food();

    void update(float dt, float floorY);
    void draw(float floorY);

    _ModelLoaderMD2* model;
    _physicsobject physics;
    _texLoader texture;

    float collisionRadius = 2.0f;

    struct {
        float y = 0;
    } rot;
};

#endif // _FOOD_H
