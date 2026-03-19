#ifndef _MODELRAT_H
#define _MODELRAT_H

#include "_modelobj.h"
#include <string>
#include <glm/glm.hpp>

class _modelRat : public _modelobj {
public:
    _modelRat(const std::string& filename, float scale = 1.0f);
    ~_modelRat();

    void drawModel() override;  // Override base class drawModel to use _modelRat's pos/rot

    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 velocity;
};

#endif
