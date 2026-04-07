#ifndef _TEXLOADER_H
#define _TEXLOADER_H

#include<_common.h>
#include<SOIL.h>

class _texLoader
{
    public:
        _texLoader();
        virtual ~_texLoader();
        int width,height; // keep track of image dimensions
        unsigned char* image;
        GLuint texID;

        void loadTexture(const char*);
        void BindTex();

    protected:

    private:
};

#endif // _TEXLOADER_H
