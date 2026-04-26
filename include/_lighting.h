#ifndef _LIGHTING_H
#define _LIGHTING_H

#include<_common.h>

class _lighting
{
    public:
        _lighting();
        virtual ~_lighting();

        const GLfloat ambientLight[4]= {0.3,0.3,0.3,1.0}; // environment light
        const GLfloat diffuseLight[4]= {0.8,0.8,0.8,1.0}; // shadow light
        const GLfloat specularLight[4]= {1.0,1.0,1.0,1.0}; // shiny light
        const GLfloat lightPosition[4]= {0.0,20.0,0.0,1.0}; // position light

        const GLfloat ambientMat[4]= {1.0,1.0,1.0,1.0}; // environment material
        const GLfloat diffuseMat[4]= {1.0,1.0,1.0,1.0}; // diffuse material
        const GLfloat specularMat[4]= {1.0,1.0,1.0,1.0}; // specular material
        const GLfloat shininess[1]= {100.0}; //environment light

       void setLight(GLenum);

    protected:

    private:
};

#endif // _LIGHTING_H
