// ================================================================
//  _food.cpp  –  Billboard food item
// ================================================================
#include "_food.h"

_food::_food(const std::string& texturePath, float dispSize, int type)
{
    foodType        = type;
    displaySize     = dispSize;
    collisionRadius = dispSize * 0.42f;

    texture = new _texLoader();
    texture->loadTexture(texturePath.c_str());
}

_food::~_food()
{
    delete texture;
}

void _food::update(float dt, float floorY)
{
    // Push position down to floor + collision radius
    physics.updatePhysics(dt, floorY + collisionRadius);
}
