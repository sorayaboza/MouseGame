#ifndef _MODELOBJ_H
#define _MODELOBJ_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>

/*
    Represents a 3D OBJ model.

    Handles:
    - loading vertices, normals, UVs from OBJ
    - creating OpenGL VAO/VBO/EBO buffers
    - drawing the model in the world
*/
class _modelobj {
public:
    _modelobj();
    ~_modelobj();

    bool loadOBJ(const std::string& filename, float scale = 1.0f); // load the OBJ file
    virtual void drawModel();   // Model render. virtual so subclasses can override

    glm::vec3 pos;        // current position of the model in world space
    glm::vec3 rot;        // rotation (Euler angles) of the model
    float modelScale;     // scale factor applied during loading

private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::string objFile;

    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    void setupBuffers();
    void normalizeModel(float desiredSize = 5.0f);

protected:
    bool buffersInitialized = false;
    GLuint vao, vboVertices, vboNormals, vboUVs, ebo;
    std::vector<unsigned int> indices;
};

#endif // _MODELOBJ_H
