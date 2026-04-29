#ifndef _FOOD_H
#define _FOOD_H

#include <string>
#include <_physicsobject.h>
#include <_texloader.h>

// ================================================================
//  _food  –  A food item rendered as a textured billboard
//
//  foodType: 0=banana, 1=cheese, 2=donut, 3=milk, 4=watermelon
// ================================================================
class _food {
public:
    _food(const std::string& texturePath, float displaySize, int type);
    ~_food();

    void update(float dt, float floorY);

    _texLoader* texture;     // billboard texture
    int         foodType;
    float       displaySize; // height of the billboard quad
    float       collisionRadius;

    _physicsobject physics;

    struct { float y = 0; } rot;  // (kept for compat – not used by billboard)
};

#endif // _FOOD_H
