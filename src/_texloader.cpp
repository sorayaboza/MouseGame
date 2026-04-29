#include "_texloader.h"

_texLoader::_texLoader() { /*ctor*/ }
_texLoader::~_texLoader() { /*dtor*/ }

void _texLoader::loadTexture(const char* fileName) {
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    image = SOIL_load_image(fileName, &width, &height, 0, SOIL_LOAD_RGBA);
    if (!image) std::cout << "ERROR: image not found: " << fileName << std::endl;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);

    // ── Sharp filtering WITHOUT mipmaps ──
    // Mipmaps + transparent PNG edges create dark halos because the
    // generated mip levels average colour with alpha=0 transparent pixels,
    // and alpha-test then rejects everything → black food at distance.
    // Plain GL_LINEAR keeps full alpha at edges so PNGs render clean.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Anisotropic filtering still helps at oblique angles
    GLfloat maxAniso = 1.0f;
    glGetFloatv(0x84FF /*GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT*/, &maxAniso);
    if (maxAniso > 1.0f) {
        if (maxAniso > 8.0f) maxAniso = 8.0f;
        glTexParameterf(GL_TEXTURE_2D, 0x84FE /*GL_TEXTURE_MAX_ANISOTROPY_EXT*/, maxAniso);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void _texLoader::BindTex() {
    glBindTexture(GL_TEXTURE_2D, texID);
}
