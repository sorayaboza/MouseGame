#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "_modelobj.h"

#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cfloat>

/*
    Generic OBJ model loader and renderer.

    Uses:
    - tiny_obj_loader to read OBJ geometry from file
    - OpenGL Vertex Buffer Objects (VBOs) to store vertex data on the GPU
    - Vertex Array Object (VAO) to group buffers for drawing
    - Supports normalization (centering + scaling)
*/

_modelobj::_modelobj() {
    // Constructor: initialize default values
    buffersInitialized = false;
}

_modelobj::~_modelobj() {
    // Destructor: clean up GPU buffers if they were created
    if (buffersInitialized) {
        glDeleteBuffers(1, &vboVertices);
        glDeleteBuffers(1, &vboNormals);
        glDeleteBuffers(1, &vboUVs);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
    }
}

/* loadOBJ()

    Loads an OBJ file using tinyobjloader and stores the vertex data.

    Steps:
    1. Read vertices, normals, and texture coordinates from file
    2. Fill indices for rendering
    3. Set up OpenGL buffers
    4. Normalize model (center + scale)
*/
bool _modelobj::loadOBJ(const std::string& filename, float scale) {
    objFile = filename;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Load OBJ file from disk
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, objFile.c_str())) {
        std::cerr << warn << err << std::endl;
        return false;
    }

    // Clear any existing data
    vertices.clear();
    normals.clear();
    uvs.clear();
    indices.clear();

    for (const auto& shape : shapes) { // Loop through shapes
        for (const auto& index : shape.mesh.indices) { // Loop through vertex indices

            // ---------------- VERTEX POSITION ----------------
            vertices.push_back({
                attrib.vertices[3*index.vertex_index+0] * scale,
                attrib.vertices[3*index.vertex_index+1] * scale,
                attrib.vertices[3*index.vertex_index+2] * scale
            });

            // ---------------- NORMAL VECTOR ----------------
            if (!attrib.normals.empty()) {
                normals.push_back({
                    attrib.normals[3*index.normal_index+0],
                    attrib.normals[3*index.normal_index+1],
                    attrib.normals[3*index.normal_index+2]
                });
            } else {
                normals.push_back({0,1,0}); // Default normal
            }

            // ---------------- TEXTURE COORDINATES ----------------
            if (!attrib.texcoords.empty() &&
                index.texcoord_index >= 0 &&
                (2 * index.texcoord_index + 1) < attrib.texcoords.size())
            {
                uvs.push_back({
                    attrib.texcoords[2*index.texcoord_index+0],
                    attrib.texcoords[2*index.texcoord_index+1]
                });
            }
            else {
                uvs.push_back({0.0f, 0.0f}); // safe fallback
            }
            if (index.texcoord_index < 0 ||
                (2 * index.texcoord_index + 1) >= attrib.texcoords.size()) {
                std::cout << "BAD UV INDEX in " << filename << std::endl;
            }

            // ---------------- ELEMENT BUFFER ----------------
            indices.push_back(indices.size());
        }
    }

    // Set up OpenGL buffers for rendering
    setupBuffers();

    // Center and scale model so it fits the scene
    normalizeModel(1.0f); // 1.0f to keep consistent model size

    return true;
}

/* setupBuffers()

    Uploads vertex, normal, UV, and index data to GPU.

    Creates:
    - VAO for grouping buffers
    - VBOs for vertices, normals, UVs
    - EBO for indices
*/
void _modelobj::setupBuffers() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // -------- VERTEX BUFFER --------
    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    // -------- NORMAL BUFFER --------
    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, 0);

    // -------- TEXTURE COORDINATE BUFFER --------
    glGenBuffers(1, &vboUVs);
    glBindBuffer(GL_ARRAY_BUFFER, vboUVs);
    glBufferData(GL_ARRAY_BUFFER, uvs.size()*sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, vboUVs); // bind before setting pointer
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)0);

    // -------- ELEMENT BUFFER --------
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind VAO
    buffersInitialized = true;
}

// Renders the obj model.
void _modelobj::drawModel() {
    if (!buffersInitialized) return;

    glPushMatrix(); // Save current matrix so other objects aren’t affected

    // 3. Draw the model geometry
    glBindVertexArray(vao);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glPopMatrix(); // Restore matrix for next object
}

/* normalizeModel()

    Centers and scales the model so it fits in the scene.

    Steps:
    1. Compute bounding box
    2. Find center
    3. Determine largest dimension
    4. Scale vertices to desired size
*/
void _modelobj::normalizeModel(float desiredSize) {
    minBounds = {FLT_MAX, FLT_MAX, FLT_MAX};
    maxBounds = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    // Compute bounding box
    for (auto &v : vertices) {
        minBounds.x = std::min(minBounds.x, v.x);
        minBounds.y = std::min(minBounds.y, v.y);
        minBounds.z = std::min(minBounds.z, v.z);

        maxBounds.x = std::max(maxBounds.x, v.x);
        maxBounds.y = std::max(maxBounds.y, v.y);
        maxBounds.z = std::max(maxBounds.z, v.z);
    }

    // Find center
    glm::vec3 center;
    center.x = (minBounds.x + maxBounds.x) / 2.0f;
    center.y = (minBounds.y + maxBounds.y) / 2.0f;
    center.z = (minBounds.z + maxBounds.z) / 2.0f;

    // Determine scale
    float sizeX = maxBounds.x - minBounds.x;
    float sizeY = maxBounds.y - minBounds.y;
    float sizeZ = maxBounds.z - minBounds.z;

    float maxSize = std::max(sizeX, std::max(sizeY, sizeZ));
    float scaleFactor = desiredSize / maxSize;

    // Re-center and scale all vertices
    for (auto &v : vertices) {
        v.x = (v.x - center.x) * scaleFactor;
        v.y = (v.y - center.y) * scaleFactor;
        v.z = (v.z - center.z) * scaleFactor;
    }
}
