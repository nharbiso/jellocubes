#include "primitives.h"
#include <algorithm>

// Inserts a 3D vector into the given vector of data
template <int dim>
void insertVec(std::vector<GLfloat>& data, glm::vec<dim, float, glm::defaultp> dataPoint) {
    for(int i = 0; i < dim; i++)
        data.push_back(dataPoint[i]);
}

void Cube::makeTile(std::vector<glm::vec3>& vertices,
                    std::vector<glm::vec3>& normals,
                    std::vector<glm::vec2>& uvs) {
    const int numVerts = 6;
    std::vector<int> vertInds = {0, 2, 3, 0, 3, 1};
    if(this->inverted) {
        vertInds = {3, 2, 0, 1, 3, 0};
    }
    for(int ind : vertInds) {
        insertVec(this->vertexData, vertices[ind]);
        insertVec(this->vertexData, normals[ind]);
        SceneFileMap& texture = this->material.textureMap;
        insertVec(this->vertexData, uvs[ind] * glm::vec2(texture.repeatU, texture.repeatV));
    }
}

const void Cube::makeFace(glm::vec3 topLeft, glm::vec3 topRight,
                          glm::vec3 bottomLeft, glm::vec3 bottomRight,
                          getNormalFunc getNormal, getUVCartFunc getUV) {
    glm::vec3 tileWidth = (topRight - topLeft) / (float) this->param;
    glm::vec3 tileHeight = (bottomLeft - topLeft) / (float) this->param;
    for(int i = 0; i < this->param; i++) {
        for(int j = 0; j < this->param; j++) {
            glm::vec3 tileTopLeft = topLeft + (float) i * tileWidth + (float) j * tileHeight;
            std::vector<glm::vec3> vertices = {
                tileTopLeft,
                tileTopLeft + tileWidth,
                tileTopLeft + tileHeight,
                tileTopLeft + tileWidth + tileHeight
            };
            std::vector<glm::vec3> normals(vertices.size());
            std::transform(vertices.begin(), vertices.end(), normals.begin(), getNormal);
            std::vector<glm::vec2> uvs(vertices.size());
            std::transform(vertices.begin(), vertices.end(), uvs.begin(), getUV);
            this->makeTile(vertices, normals, uvs);
        }
    }
}

const void Cube::calcVertexData() {
    this->vertexData.clear();
    // +x face
    makeFace(glm::vec3(this->boundCoord,  this->boundCoord,  this->boundCoord),
             glm::vec3(this->boundCoord,  this->boundCoord, -this->boundCoord),
             glm::vec3(this->boundCoord, -this->boundCoord,  this->boundCoord),
             glm::vec3(this->boundCoord, -this->boundCoord, -this->boundCoord),
             [this](glm::vec3 pos) -> glm::vec3 { return glm::vec3(1 * (this->inverted ? -1 : 1), 0, 0); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(0.5 - pos.z, pos.y + 0.5); });

    // -x face
    makeFace(glm::vec3(-this->boundCoord,  this->boundCoord, -this->boundCoord),
             glm::vec3(-this->boundCoord,  this->boundCoord,  this->boundCoord),
             glm::vec3(-this->boundCoord, -this->boundCoord, -this->boundCoord),
             glm::vec3(-this->boundCoord, -this->boundCoord,  this->boundCoord),
             [this](glm::vec3 pos) -> glm::vec3 { return glm::vec3(-1 * (this->inverted ? -1 : 1), 0, 0); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(pos.z + 0.5, pos.y + 0.5); });

    // +y face
    makeFace(glm::vec3( this->boundCoord, this->boundCoord,  this->boundCoord),
             glm::vec3(-this->boundCoord, this->boundCoord,  this->boundCoord),
             glm::vec3( this->boundCoord, this->boundCoord, -this->boundCoord),
             glm::vec3(-this->boundCoord, this->boundCoord, -this->boundCoord),
             [this](glm::vec3 pos) -> glm::vec3 { return glm::vec3(0, 1 * (this->inverted ? -1 : 1), 0); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(pos.x + 0.5, 0.5 - pos.z); });

    // -y face
    makeFace(glm::vec3(-this->boundCoord, -this->boundCoord,  this->boundCoord),
             glm::vec3( this->boundCoord, -this->boundCoord,  this->boundCoord),
             glm::vec3(-this->boundCoord, -this->boundCoord, -this->boundCoord),
             glm::vec3( this->boundCoord, -this->boundCoord, -this->boundCoord),
             [this](glm::vec3 pos) -> glm::vec3 { return glm::vec3(0, -1 * (this->inverted ? -1 : 1), 0); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(pos.x + 0.5, pos.z + 0.5); });

    // +z face
    makeFace(glm::vec3(-this->boundCoord,  this->boundCoord, this->boundCoord),
             glm::vec3( this->boundCoord,  this->boundCoord, this->boundCoord),
             glm::vec3(-this->boundCoord, -this->boundCoord, this->boundCoord),
             glm::vec3( this->boundCoord, -this->boundCoord, this->boundCoord),
             [this](glm::vec3 pos) -> glm::vec3 { return glm::vec3(0, 0, 1 * (this->inverted ? -1 : 1)); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(pos.x + 0.5, pos.y + 0.5); });

    // -z face
    makeFace(glm::vec3( this->boundCoord,  this->boundCoord, -this->boundCoord),
             glm::vec3(-this->boundCoord,  this->boundCoord, -this->boundCoord),
             glm::vec3( this->boundCoord, -this->boundCoord, -this->boundCoord),
             glm::vec3(-this->boundCoord, -this->boundCoord, -this->boundCoord),
             [this](glm::vec3 pos) -> glm::vec3 { return glm::vec3(0, 0, -1 * (this->inverted ? -1 : 1)); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(0.5 - pos.x, pos.y + 0.5); });
}
