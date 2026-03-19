#include "_texloader.h"

_texLoader::_texLoader() { /*ctor*/ }

_texLoader::~_texLoader() { /*dtor*/ }

void _texLoader::loadTexture(char* fileName) {
    glGenTextures(1,&texID);      // create handle
    glBindTexture(GL_TEXTURE_2D,texID); // bind handle to buffer

    image = SOIL_load_image(fileName, &width,&height,0,SOIL_LOAD_RGBA);
    if(!image)cout<<"ERORR: The image file not found"<<endl;

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);

     glBindTexture(GL_TEXTURE_2D,0); // bind handle to buffer
}

void _texLoader::BindTex() {
   glBindTexture(GL_TEXTURE_2D,texID);
}
