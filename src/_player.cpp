#include "_player.h"
#include <GL/glew.h>

_player::_player() {
    model    = new _ModelLoaderMD2();
    isMoving = false;
    animTime = 0.0f;
}
_player::~_player() {
    // model is managed by scene
}

void _player::init(const std::string& modelPath, const std::string& texturePath) {
    model->initModel(modelPath.c_str(), (char*)texturePath.c_str());
}

void _player::update(float dt) {
    if (isMoving) animTime += dt;
}
