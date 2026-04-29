#ifndef _LEVELTHEME_H
#define _LEVELTHEME_H

#include "_skybox.h"
#include "_player.h"
#include "_enemy.h"
#include <vector>

class _leveltheme {
public:
    _leveltheme();
    ~_leveltheme();

    void apply(int level,
               _skyBox* sky,
               _player* player,
               _enemy* cat,
               std::vector<_enemy*>& dogs);
};

#endif
