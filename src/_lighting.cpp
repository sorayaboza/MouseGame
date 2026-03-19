#include "_lighting.h"

_lighting::_lighting() { /*ctor*/ }

_lighting::~_lighting() { /*dtor*/ }

void _lighting::setLight(GLenum light) {
    glLightfv(light, GL_AMBIENT, ambientLight);
    glLightfv(light, GL_DIFFUSE, diffuseLight);
    glLightfv(light, GL_SPECULAR,specularLight);
    glLightfv(light, GL_POSITION, lightPosition);

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMat);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMat);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specularMat);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}
