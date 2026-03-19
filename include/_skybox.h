#ifndef _SKYBOX_H
#define _SKYBOX_H

#include<_common.h>
#include<_texloader.h>

class _skyBox
{
    public:
        _skyBox();
        virtual ~_skyBox();

        vec3 rot;
        vec3 pos;
        vec3 scale;

        void drawBox();
        void boxInit();

        _texLoader myTex[6];

        GLuint vboVert, vboNorm,vboTex, eboID;

         // 24 vertices (4 per face) to allow for unique face normals
    GLfloat vertices[72] = {
        -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, // Front
        -0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f,-0.5f,-0.5f, // Back
        -0.5f, 0.5f,-0.5f, -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f, // Top
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, // Bottom
         0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f, // Right
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f  // Left
    };

    // Separate array for normals
    GLfloat normals[72] = {
         0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f, // Front
         0.0f, 0.0f,-1.0f,  0.0f, 0.0f,-1.0f,  0.0f, 0.0f,-1.0f,  0.0f, 0.0f,-1.0f, // Back
         0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f, // Top
         0.0f,-1.0f, 0.0f,  0.0f,-1.0f, 0.0f,  0.0f,-1.0f, 0.0f,  0.0f,-1.0f, 0.0f, // Bottom
         1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f, // Right
        -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f  // Left
    };


    GLfloat texCoords[48] = {
    // Front: (Bottom-Left, Bottom-Right, Top-Right, Top-Left)
    0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
    // Back: (Bottom-Right, Top-Right, Top-Left, Bottom-Left)
    1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,
    // Top: (Back-Left, Front-Left, Front-Right, Back-Right)
    1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,
    // Bottom: (Back-Left, Back-Right, Front-Right, Front-Left)
    1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
    // Right: (Back-Bottom, Back-Top, Front-Top, Front-Bottom)
    1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0.0f, 1.0f,
    // Left: (Back-Bottom, Front-Bottom, Front-Top, Back-Top)
    0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f
   };


    GLuint indices[36] = {
        0, 1, 2, 0, 2, 3,       4, 5, 6, 4, 6, 7,       8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23
    };



    protected:

    private:
};

#endif // _SKYBOX_H
