#include "primitives.h"
#include <algorithm>

// Inserts a 3D vector into the given vector of data
template <int dim>
void insertVec(std::vector<GLfloat>& data, glm::vec<dim, float, glm::defaultp> dataPoint) {
    for(int i = 0; i < dim; i++)
        data.push_back(dataPoint[i]);
}

inline glm::vec3 sphericalToCartesian(float radius, float theta, float phi) {
    return radius * glm::vec3(glm::sin(phi) * glm::cos(theta), glm::cos(phi), glm::sin(phi) * glm::sin(theta));
}

void TessellatedPrimitive::makeTile(std::vector<glm::vec3>& vertices,
                                    std::vector<glm::vec3>& normals,
                                    std::vector<glm::vec2>& uvs,
                                    bool inverted) {
    const int numVerts = 6;
    std::vector<int> vertInds = {0, 2, 3, 0, 3, 1};
    if(inverted) {
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
    glm::vec3 tileWidth = (topRight - topLeft) / (float) this->param1;
    glm::vec3 tileHeight = (bottomLeft - topLeft) / (float) this->param1;
    for(int i = 0; i < this->param1; i++) {
        for(int j = 0; j < this->param1; j++) {
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
            this->makeTile(vertices, normals, uvs, this->inverted);
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

const std::optional<glm::vec3> Cube::findIntersectionPoint(glm::vec3 point) {
    glm::vec4 objSpacePoint = this->worldToObject * glm::vec4(point, 1);
    if(-0.5 <= objSpacePoint.x && objSpacePoint.x <= 0.5 &&
       -0.5 <= objSpacePoint.y && objSpacePoint.y <= 0.5 &&
        -0.5 <= objSpacePoint.z && objSpacePoint.z <= 0.5) {
        // point is inside cube
        glm::vec4 interPoint(-0.5, objSpacePoint.y, objSpacePoint.z, 1);
        float minDist = objSpacePoint.x + 0.5;
        if(0.5 - objSpacePoint.x < minDist) {
            interPoint = glm::vec4(0.5, objSpacePoint.y, objSpacePoint.z, 1);
            minDist = 0.5 - objSpacePoint.x;
        }
        if(objSpacePoint.y + 0.5 < minDist) {
            interPoint = glm::vec4(objSpacePoint.x, -0.5, objSpacePoint.z, 1);
            minDist = objSpacePoint.y + 0.5;
        }
        if(0.5 - objSpacePoint.y < minDist) {
            interPoint = glm::vec4(objSpacePoint.x, 0.5, objSpacePoint.z, 1);
            minDist = 0.5 - objSpacePoint.y;
        }
        if(objSpacePoint.z + 0.5 < minDist) {
            interPoint = glm::vec4(objSpacePoint.x, objSpacePoint.y, -0.5, 1);
            minDist = objSpacePoint.z + 0.5;
        }
        if(0.5 - objSpacePoint.z < minDist) {
            interPoint = glm::vec4(objSpacePoint.x, objSpacePoint.y, 0.5, 1);
        }
        return this->objectToWorld * interPoint;
    }
    return std::nullopt;
}


const void Sphere::calcVertexData() {
    this->vertexData.clear();
    float thetaStep = glm::radians(360.0f / this->param2);
    float phiStep = glm::radians(180.0f / this->param1);
    for(int i = 0; i < this->param2; i++) {
        float currTheta = i * thetaStep;
        for(int j = 0; j < this->param1; j++) {
            float currPhi = phiStep * j;
            std::vector<glm::vec3> vertices = {
                sphericalToCartesian(this->radius, currTheta + thetaStep, currPhi),
                sphericalToCartesian(this->radius, currTheta, currPhi),
                sphericalToCartesian(this->radius, currTheta + thetaStep, currPhi + phiStep),
                sphericalToCartesian(this->radius, currTheta, currPhi + phiStep)
            };
            std::vector<glm::vec3> normals(vertices.size());
            std::transform(vertices.begin(), vertices.end(), normals.begin(), this->getSphereNormal);
            std::vector<glm::vec2> uvs = {
                this->getSphereUV(this->radius, currTheta + thetaStep, currPhi),
                this->getSphereUV(this->radius, currTheta, currPhi),
                this->getSphereUV(this->radius, currTheta + thetaStep, currPhi + phiStep),
                this->getSphereUV(this->radius, currTheta, currPhi + phiStep)
            };
            this->makeTile(vertices, normals, uvs, false);
        }
    }
}

const std::optional<glm::vec3> Sphere::findIntersectionPoint(glm::vec3 point) {
    glm::vec3 objSpacePoint = this->worldToObject * glm::vec4(point, 1);
    if(glm::length(objSpacePoint) <= 0.5) {
        // point is inside sphere
        glm::vec4 interPoint = glm::vec4(glm::normalize(objSpacePoint) * 0.5f, 1);
        return this->objectToWorld * interPoint;
    }
    return std::nullopt;
}
