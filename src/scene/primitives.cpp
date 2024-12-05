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

inline glm::vec3 cylindricalToCartesian(float radius, float theta, float z) {
    return glm::vec3(radius * cos(theta), z, radius * sin(theta));
}

void TessellatedPrimitive::makeTile(std::vector<glm::vec3>& vertices,
                                    std::vector<glm::vec3>& normals,
                                    std::vector<glm::vec2>& uvs) {
    const int numVerts = 6;
    int vertInds[numVerts] = {0, 2, 3, 0, 3, 1};
    for(int ind : vertInds) {
        insertVec(this->vertexData, vertices[ind]);
        insertVec(this->vertexData, normals[ind]);
        SceneFileMap& texture = this->material.textureMap;
        insertVec(this->vertexData, uvs[ind] * glm::vec2(texture.repeatU, texture.repeatV));
    }
}

int TessellatedPrimitive::adjustParam(int param, int minParam, TessellationParams& tslParams) {
    float paramf = param;
    const std::size_t startPrim = 5;
    if(tslParams.limitByNum && tslParams.numPrimitives > startPrim) {
        // Adjust parameter by inverse number of primitives in scene
        paramf /= sqrt(tslParams.numPrimitives - startPrim + 1);
    }

    if(tslParams.limitByDist) {
        // Convert camera to object space
        glm::vec4 objCameraPos = this->worldToObject * tslParams.cameraPos;
        // Find distance vector from camera to [-0.5, 0.5]^3 bounding cube of primitive
        glm::vec4 objDist(
            std::max({-0.5f - objCameraPos.x, objCameraPos.x - 0.5f, 0.0f}),
            std::max({-0.5f - objCameraPos.y, objCameraPos.y - 0.5f, 0.0f}),
            std::max({-0.5f - objCameraPos.z, objCameraPos.z - 0.5f, 0.0f}),
            0
        );
        // Convert distance vector back to world space and find its length
        float dist = glm::length(this->objectToWorld * objDist);
        // Based on near and far plane distances, determine how to scale parameter
        paramf *= std::clamp((tslParams.farPlane - dist) / (tslParams.farPlane - tslParams.nearPlane), 0.0f, 1.0f);
    }
    return std::max((int) round(paramf), minParam);
}


const void Sphere::calcVertexData() {
    this->vertexData.clear();
    float thetaStep = glm::radians(360.0f / this->adjParam2);
    float phiStep = glm::radians(180.0f / this->adjParam1);
    for(int i = 0; i < this->adjParam2; i++) {
        float currTheta = i * thetaStep;
        for(int j = 0; j < this->adjParam1; j++) {
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
            this->makeTile(vertices, normals, uvs);
        }
    }
}



const void Cube::makeFace(glm::vec3 topLeft, glm::vec3 topRight,
                          glm::vec3 bottomLeft, glm::vec3 bottomRight,
                          getNormalFunc getNormal, getUVCartFunc getUV) {
    glm::vec3 tileWidth = (topRight - topLeft) / (float) this->adjParam1;
    glm::vec3 tileHeight = (bottomLeft - topLeft) / (float) this->adjParam1;
    for(int i = 0; i < this->adjParam1; i++) {
        for(int j = 0; j < this->adjParam1; j++) {
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
             [](glm::vec3 pos) -> glm::vec3 { return glm::vec3(1, 0, 0); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(0.5 - pos.z, pos.y + 0.5); });

    // -x face
    makeFace(glm::vec3(-this->boundCoord,  this->boundCoord, -this->boundCoord),
             glm::vec3(-this->boundCoord,  this->boundCoord,  this->boundCoord),
             glm::vec3(-this->boundCoord, -this->boundCoord, -this->boundCoord),
             glm::vec3(-this->boundCoord, -this->boundCoord,  this->boundCoord),
             [](glm::vec3 pos) -> glm::vec3 { return glm::vec3(-1, 0, 0); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(pos.z + 0.5, pos.y + 0.5); });

    // +y face
    makeFace(glm::vec3( this->boundCoord, this->boundCoord,  this->boundCoord),
             glm::vec3(-this->boundCoord, this->boundCoord,  this->boundCoord),
             glm::vec3( this->boundCoord, this->boundCoord, -this->boundCoord),
             glm::vec3(-this->boundCoord, this->boundCoord, -this->boundCoord),
             [](glm::vec3 pos) -> glm::vec3 { return glm::vec3(0, 1, 0); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(pos.x + 0.5, 0.5 - pos.z); });

    // -y face
    makeFace(glm::vec3(-this->boundCoord, -this->boundCoord,  this->boundCoord),
             glm::vec3( this->boundCoord, -this->boundCoord,  this->boundCoord),
             glm::vec3(-this->boundCoord, -this->boundCoord, -this->boundCoord),
             glm::vec3( this->boundCoord, -this->boundCoord, -this->boundCoord),
             [](glm::vec3 pos) -> glm::vec3 { return glm::vec3(0, -1, 0); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(pos.x + 0.5, pos.z + 0.5); });

    // +z face
    makeFace(glm::vec3(-this->boundCoord,  this->boundCoord, this->boundCoord),
             glm::vec3( this->boundCoord,  this->boundCoord, this->boundCoord),
             glm::vec3(-this->boundCoord, -this->boundCoord, this->boundCoord),
             glm::vec3( this->boundCoord, -this->boundCoord, this->boundCoord),
             [](glm::vec3 pos) -> glm::vec3 { return glm::vec3(0, 0, 1); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(pos.x + 0.5, pos.y + 0.5); });

    // -z face
    makeFace(glm::vec3( this->boundCoord,  this->boundCoord, -this->boundCoord),
             glm::vec3(-this->boundCoord,  this->boundCoord, -this->boundCoord),
             glm::vec3( this->boundCoord, -this->boundCoord, -this->boundCoord),
             glm::vec3(-this->boundCoord, -this->boundCoord, -this->boundCoord),
             [](glm::vec3 pos) -> glm::vec3 { return glm::vec3(0, 0, -1); },
             [](glm::vec3 pos) -> glm::vec2 { return glm::vec2(0.5 - pos.x, pos.y + 0.5); });
}



const void Cylinder::calcVertexData() {
    this->vertexData.clear();
    float baseY = -this->height / 2;
    float topY = this->height / 2;
    float thetaStep = glm::radians(360.0f / this->adjParam2);
    float heightStep = this->height / this->adjParam1;
    float radStep = this->radius / this->adjParam1;
    for(int i = 0; i < this->adjParam2; i++) {
        float currTheta = i * thetaStep;

        // Tessellate side
        for(int j = 0; j < this->adjParam1; j++) {
            float currHeight = baseY + heightStep * j;
            std::vector<glm::vec3> vertices = {
                cylindricalToCartesian(this->radius, currTheta + thetaStep, currHeight + heightStep),
                cylindricalToCartesian(this->radius, currTheta, currHeight + heightStep),
                cylindricalToCartesian(this->radius, currTheta + thetaStep, currHeight),
                cylindricalToCartesian(this->radius, currTheta, currHeight)
            };
            std::vector<glm::vec3> normals(vertices.size());
            std::transform(vertices.begin(), vertices.end(), normals.begin(), this->getSideNormal);
            std::vector<glm::vec2> uvs = {
                this->getSideUV(this->radius, currTheta + thetaStep, currHeight + heightStep),
                this->getSideUV(this->radius, currTheta, currHeight + heightStep),
                this->getSideUV(this->radius, currTheta + thetaStep, currHeight),
                this->getSideUV(this->radius, currTheta, currHeight)
            };
            this->makeTile(vertices, normals, uvs);
        }

        // Tessellate base
        for(int j = 0; j < this->adjParam1; j++) {
            float currRadius = radStep * j;
            std::vector<glm::vec3> vertices = {
                cylindricalToCartesian(currRadius, currTheta, baseY),
                cylindricalToCartesian(currRadius, currTheta + thetaStep, baseY),
                cylindricalToCartesian(currRadius + radStep, currTheta, baseY),
                cylindricalToCartesian(currRadius + radStep, currTheta + thetaStep, baseY)
            };
            std::vector<glm::vec3> normals(vertices.size());
            std::transform(vertices.begin(), vertices.end(), normals.begin(), this->getBaseNormal);
            std::vector<glm::vec2> uvs(vertices.size());
            std::transform(vertices.begin(), vertices.end(), uvs.begin(), this->getBaseUV);
            this->makeTile(vertices, normals, uvs);
        }

        // Tessellate top
        for(int j = 0; j < this->adjParam1; j++) {
            float currRadius = radStep * j;
            std::vector<glm::vec3> vertices = {
                cylindricalToCartesian(currRadius, currTheta + thetaStep, topY),
                cylindricalToCartesian(currRadius, currTheta, topY),
                cylindricalToCartesian(currRadius + radStep, currTheta + thetaStep, topY),
                cylindricalToCartesian(currRadius + radStep, currTheta, topY),
            };
            std::vector<glm::vec3> normals(vertices.size());
            std::transform(vertices.begin(), vertices.end(), normals.begin(), this->getTopNormal);
            std::vector<glm::vec2> uvs(vertices.size());
            std::transform(vertices.begin(), vertices.end(), uvs.begin(), this->getTopUV);
            this->makeTile(vertices, normals, uvs);
        }
    }
}


const void Cone::calcVertexData() {
    this->vertexData.clear();
    float baseY = -this->height / 2;
    float thetaStep = glm::radians(360.0f / this->adjParam2);
    float heightStep = this->height / this->adjParam1;
    float radStep = this->baseRadius / this->adjParam1;
    for(int i = 0; i < this->adjParam2; i++) {
        float currTheta = i * thetaStep;

        // Tessellate side
        for(int j = 0; j < this->adjParam1; j++) {
            float bottomH = baseY + heightStep * j;
            float topRadius = 0.25f - (bottomH + heightStep) / 2;
            float bottomRadius = 0.25f - bottomH / 2;
            std::vector<glm::vec3> vertices = {
                cylindricalToCartesian(topRadius, currTheta + thetaStep, bottomH + heightStep),
                cylindricalToCartesian(topRadius, currTheta, bottomH + heightStep),
                cylindricalToCartesian(bottomRadius, currTheta + thetaStep, bottomH),
                cylindricalToCartesian(bottomRadius, currTheta, bottomH)
            };
            std::vector<glm::vec3> normals(vertices.size());
            std::transform(vertices.begin(), vertices.end(), normals.begin(), this->getSideNormal);
            if(j == this->adjParam1 - 1) { // tip of cone, normal is average of bottom two
                glm::vec3 avg = normalize((normalize(normals[2]) + normalize(normals[3])) / 2.0f);
                normals[0] = avg;
                normals[1] = avg;
            }
            std::vector<glm::vec2> uvs = {
                this->getSideUV(topRadius, currTheta + thetaStep, bottomH + heightStep),
                this->getSideUV(topRadius, currTheta, bottomH + heightStep),
                this->getSideUV(bottomRadius, currTheta + thetaStep, bottomH),
                this->getSideUV(bottomRadius, currTheta, bottomH)
            };
            this->makeTile(vertices, normals, uvs);
        }

        // Tessellate base
        for(int j = 0; j < this->adjParam1; j++) {
            float currRadius = radStep * j;
            std::vector<glm::vec3> vertices = {
                cylindricalToCartesian(currRadius, currTheta, baseY),
                cylindricalToCartesian(currRadius, currTheta + thetaStep, baseY),
                cylindricalToCartesian(currRadius + radStep, currTheta, baseY),
                cylindricalToCartesian(currRadius + radStep, currTheta + thetaStep, baseY),
            };
            std::vector<glm::vec3> normals(vertices.size());
            std::transform(vertices.begin(), vertices.end(), normals.begin(), this->getBaseNormal);
            std::vector<glm::vec2> uvs(vertices.size());
            std::transform(vertices.begin(), vertices.end(), uvs.begin(), this->getBaseUV);
            this->makeTile(vertices, normals, uvs);
        }
    }
}
