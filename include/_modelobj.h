#ifndef _MODELOBJ_H
#define _MODELOBJ_H

#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

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

    bool loadOBJ(const std::string& filename, float scale);
    void drawModel();

private:
    void setupBuffers();
    void normalizeModel(float desiredSize);

    std::string objFile;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;

    glm::vec3 minBounds, maxBounds;

    unsigned int vao, vboVertices, vboNormals, vboUVs, ebo;
    bool buffersInitialized;
};

#endif // _MODELOBJ_H
