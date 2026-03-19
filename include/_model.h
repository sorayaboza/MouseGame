#ifndef _MODEL_H
#define _MODEL_H

#include <_common.h>
#include <_texloader.h>

class _model
{
    public:
        _model();
        virtual ~_model();

        GLUquadric *quad;   // quadric object used to draw the sphere

        vec3 pos; // for translation
        vec3 scale; //resizing the model
        vec3 rot; // rotate the model

        vec3 velocity; // physics velocity / movement speed
        vec3 acceleration; // forces like gravity

        _texLoader *myTex = new _texLoader();

        void drawModel(); // renderer
        void initModel(char *); //initialization
        void updatePhysics();

    protected:

    private:
};

#endif // _MODEL_H
