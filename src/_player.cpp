#include "_player.h"
#include <GL/glew.h>

_player::_player() {
    model = new _ModelLoaderMD2();
}

void _player::init(const std::string& modelPath) {
    model->initModel(modelPath.c_str(), (char*)"images/mouse.png");
}

void _player::update(float dt) {
    // movement handled in scene
}
