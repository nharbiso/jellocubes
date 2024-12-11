#include "jellocube.h"
#include "settings.h"
#include <omp.h>

JelloCube::JelloCube(const SceneMaterial& material, int param, glm::vec<3, double> center) : Cube(glm::mat4(1), material, param, false) {
    this->restLen = 1.0f / param;
    for(int i = 0; i <= param; i++) {
        for(int j = 0; j <= param; j++) {
            for(int k = 0; k <= param; k++) {
                nodes.push_back(center + glm::vec<3, double>(-0.5 + i * restLen, -0.5 + j * restLen, -0.5 + k * restLen));
                velocities.push_back(glm::vec<3, double>(0, 0, 0));
            }
        }
    }

    std::random_device rd;
    this->gen = std::mt19937(rd());
}

glm::vec<3, double> JelloCube::getStructuralForce(int i, int j, int k) {
    glm::vec<3, double> force(0);
    if(i > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i-1, j, k), settings.kElastic, this->restLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i-1, j, k), settings.dElastic);
    }
    if(i < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i+1, j, k), settings.kElastic, this->restLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i+1, j, k), settings.dElastic);
    }
    if(j > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j-1, k), settings.kElastic, this->restLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j-1, k), settings.dElastic);
    }
    if(j < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j+1, k), settings.kElastic, this->restLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j+1, k), settings.dElastic);
    }
    if(k > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j, k-1), settings.kElastic, this->restLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j, k-1), settings.dElastic);
    }
    if(k < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j, k+1), settings.kElastic, this->restLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j, k+1), settings.dElastic);
    }
    return force;
}

glm::vec<3, double> JelloCube::getShearForce(int i, int j, int k) {
    glm::vec<3, double> force(0);
    double axisDiagRestLen = sqrt(2) * this->restLen;
    // 4 diagonals along x-axis
    if(j > 0 && k > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j-1, k-1), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j-1, k-1), settings.dElastic);
    }
    if(j > 0 && k < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j-1, k+1), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j-1, k+1), settings.dElastic);
    }
    if(j < this->param1 && k > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j+1, k-1), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j+1, k-1), settings.dElastic);
    }
    if(j < this->param1 && k < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j+1, k+1), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j+1, k+1), settings.dElastic);
    }
    // 4 diagonals along y-axis
    if(i > 0 && k > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i-1, j, k-1), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i-1, j, k-1), settings.dElastic);
    }
    if(i > 0 && k < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i-1, j, k+1), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i-1, j, k+1), settings.dElastic);
    }
    if(i < this->param1 && k > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i+1, j, k-1), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i+1, j, k-1), settings.dElastic);
    }
    if(i < this->param1 && k < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i+1, j, k+1), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i+1, j, k+1), settings.dElastic);
    }
    // 4 diagonals along z-axis
    if(i > 0 && j > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i-1, j-1, k), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i-1, j-1, k), settings.dElastic);
    }
    if(i > 0 && j < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i-1, j+1, k), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i-1, j+1, k), settings.dElastic);
    }
    if(i < this->param1 && j > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i+1, j-1, k), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i+1, j-1, k), settings.dElastic);
    }
    if(i < this->param1 && j < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i+1, j+1, k), settings.kElastic, axisDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i+1, j+1, k), settings.dElastic);
    }
    // 8 corners
    double cornerDiagRestLen = sqrt(3) * this->restLen;
    if(i > 0 && j > 0 && k > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i-1, j-1, k-1), settings.kElastic, cornerDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i-1, j-1, k-1), settings.dElastic);
    }
    if(i > 0 && j > 0 && k < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i-1, j-1, k+1), settings.kElastic, cornerDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i-1, j-1, k+1), settings.dElastic);
    }
    if(i > 0 && j < this->param1 && k > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i-1, j+1, k-1), settings.kElastic, cornerDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i-1, j+1, k-1), settings.dElastic);
    }
    if(i > 0 && j < this->param1 && k < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i-1, j+1, k+1), settings.kElastic, cornerDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i-1, j+1, k+1), settings.dElastic);
    }
    if(i < this->param1 && j > 0 && k > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i+1, j-1, k-1), settings.kElastic, cornerDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i+1, j-1, k-1), settings.dElastic);
    }
    if(i < this->param1 && j > 0 && k < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i+1, j-1, k+1), settings.kElastic, cornerDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i+1, j-1, k+1), settings.dElastic);
    }
    if(i < this->param1 && j < this->param1 && k > 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i+1, j+1, k-1), settings.kElastic, cornerDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i+1, j+1, k-1), settings.dElastic);
    }
    if(i < this->param1 && j < this->param1 && k < this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i+1, j+1, k+1), settings.kElastic, cornerDiagRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i+1, j+1, k+1), settings.dElastic);
    }
    return force;
}

glm::vec<3, double> JelloCube::getBendForce(int i, int j, int k) {
    glm::vec<3, double> force(0);
    double bendRestLen = 2 * this->restLen;
    if(i-2 >= 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i-2, j, k), settings.kElastic, bendRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i-2, j, k), settings.dElastic);
    }
    if(i+2 <= this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i+2, j, k), settings.kElastic, bendRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i+2, j, k), settings.dElastic);
    }
    if(j-2 >= 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j-2, k), settings.kElastic, bendRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j-2, k), settings.dElastic);
    }
    if(j+2 <= this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j+2, k), settings.kElastic, bendRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j+2, k), settings.dElastic);
    }
    if(k-2 >= 0) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j, k-2), settings.kElastic, bendRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j, k-2), settings.dElastic);
    }
    if(k+2 <= this->param1) {
        force += this->hooksForce(getInd(i, j, k), getInd(i, j, k+2), settings.kElastic, bendRestLen);
        force += this->dampeningForce(getInd(i, j, k), getInd(i, j, k+2), settings.dElastic);
    }
    return force;
}

glm::vec<3, double> JelloCube::getCollisionForce(int i, int j, int k, std::span<std::unique_ptr<Primitive>>& primitives) {
    glm::vec<3, double> force(0);
    glm::vec<3, double> pos = this->nodes[getInd(i, j, k)];
    glm::vec<3, double> vel = this->velocities[getInd(i, j, k)];
    glm::vec<3, double> objVel = glm::vec<3, double>(0);
    if(pos.x > settings.bounds) {
        glm::vec<3, double> collisionPoint(settings.bounds, pos.y, pos.z);
        force += this->hooksForce(pos, collisionPoint, settings.kCollision, 0);
        force += this->dampeningForce(pos, collisionPoint, vel, objVel, settings.dCollision);
    }
    if(pos.x < -settings.bounds) {
        glm::vec<3, double> collisionPoint(-settings.bounds, pos.y, pos.z);
        force += this->hooksForce(pos, collisionPoint, settings.kCollision, 0);
        force += this->dampeningForce(pos, collisionPoint, vel, objVel, settings.dCollision);
    }
    if(pos.y > settings.bounds) {
        glm::vec<3, double> collisionPoint(pos.x, settings.bounds, pos.z);
        force += this->hooksForce(pos, collisionPoint, settings.kCollision, 0);
        force += this->dampeningForce(pos, collisionPoint, vel, objVel, settings.dCollision);
    }
    if(pos.y < -settings.bounds) {
        glm::vec<3, double> collisionPoint(pos.x, -settings.bounds, pos.z);
        force += this->hooksForce(pos, collisionPoint, settings.kCollision, 0);
        force += this->dampeningForce(pos, collisionPoint, vel, objVel, settings.dCollision);
    }
    if(pos.z > settings.bounds) {
        glm::vec<3, double> collisionPoint(pos.x, pos.y, settings.bounds);
        force += this->hooksForce(pos, collisionPoint, settings.kCollision, 0);
        force += this->dampeningForce(pos, collisionPoint, vel, objVel, settings.dCollision);
    }
    if(pos.z < -settings.bounds) {
        glm::vec<3, double> collisionPoint(pos.x, pos.y, -settings.bounds);
        force += this->hooksForce(pos, collisionPoint, settings.kCollision, 0);
        force += this->dampeningForce(pos, collisionPoint, vel, objVel, settings.dCollision);
    }
    for(std::unique_ptr<Primitive>& primitive : primitives) {
        std::optional<glm::vec3> interPoint = primitive->findIntersectionPoint(pos);
        if(interPoint) {
            glm::vec<3, double> doubleInterPoint(*interPoint);
            force += this->hooksForce(pos, doubleInterPoint, settings.kCollision, 0);
            force += this->dampeningForce(pos, doubleInterPoint, vel, objVel, settings.dCollision);
        }
    }
    return force;
}

// Computes the total acceleration for all nodes given their current positions/velocities
void JelloCube::computeAcceleration(std::vector<glm::vec<3, double>>& positions,
                                    std::vector<glm::vec<3, double>>& velocities,
                                    std::vector<glm::vec<3, double>>& acc,
                                    std::span<std::unique_ptr<Primitive>>& primitives) {
    // #pragma omp parallel for collapse(3)
    for(int i = 0; i <= this->param1; i++) {
        for(int j = 0; j <= this->param1; j++) {
            for(int k = 0; k <= this->param1; k++) {
                int ind = this->getInd(i, j, k);
                acc[ind] = glm::vec<3, double>(0);
                acc[ind] += this->getStructuralForce(i, j, k);
                acc[ind] += this->getShearForce(i, j, k);
                acc[ind] += this->getBendForce(i, j, k);
                acc[ind] += this->getCollisionForce(i, j, k, primitives);
                acc[ind] += glm::vec<3, double>(0, -settings.gravity, 0);

                acc[ind] /= settings.mass;
            }
        }
    }
}

// Updates the colors, as well as positions and velocities of the jello cube's nodes using RK4 integration
void JelloCube::update(std::span<std::unique_ptr<Primitive>>& primitives) {
    this->material.cDiffuse.a = settings.transparentCube ? 0.5 : 1;

    std::vector<glm::vec<3, double>> tmpPos(this->nodes.size()), tmpVels(this->nodes.size());
    std::vector<glm::vec<3, double>> F1pos(this->nodes.size()), F1vel(this->nodes.size());
    std::vector<glm::vec<3, double>> F2pos(this->nodes.size()), F2vel(this->nodes.size());
    std::vector<glm::vec<3, double>> F3pos(this->nodes.size()), F3vel(this->nodes.size());
    std::vector<glm::vec<3, double>> F4pos(this->nodes.size()), F4vel(this->nodes.size());
    std::vector<glm::vec<3, double>> acc(this->nodes.size());

    double dt = settings.dt / 1000.0;
    if(settings.integrator == Integrator::EULER) {
        this->computeAcceleration(this->nodes, this->velocities, acc, primitives);
        // #pragma omp parallel for collapse(3)
        for(int i = 0; i <= this->param1; i++) {
            for(int j = 0; j <= this->param1; j++) {
                for(int k = 0; k <= this->param1; k++) {
                    int ind = getInd(i, j, k);
                    this->nodes[ind] += dt * this->velocities[ind];
                    this->velocities[ind] += dt * acc[ind];
                }
            }
        }
    } else if(settings.integrator == Integrator::RK4) {
        this->computeAcceleration(this->nodes, this->velocities, acc, primitives);
        // #pragma omp parallel for collapse(3)
        for(int i = 0; i <= this->param1; i++) {
            for(int j = 0; j <= this->param1; j++) {
                for(int k = 0; k <= this->param1; k++) {
                    int ind = getInd(i, j, k);
                    F1pos[ind] = this->velocities[ind] * dt;
                    F1vel[ind] = acc[ind] * dt;

                    tmpPos[ind] = this->nodes[ind] + F1pos[ind] * 0.5;
                    tmpVels[ind] = this->velocities[ind] + F1vel[ind] * 0.5;
                }
            }
        }

        this->computeAcceleration(this->nodes, this->velocities, acc, primitives);
        // #pragma omp parallel for collapse(3)
        for(int i = 0; i <= this->param1; i++) {
            for(int j = 0; j <= this->param1; j++) {
                for(int k = 0; k <= this->param1; k++) {
                    int ind = getInd(i, j, k);
                    F2pos[ind] = tmpVels[ind] * dt;
                    F2vel[ind] = acc[ind] * dt;

                    tmpPos[ind] = this->nodes[ind] + F2pos[ind] * 0.5;
                    tmpVels[ind] = this->velocities[ind] + F2vel[ind] * 0.5;
                }
            }
        }

        this->computeAcceleration(this->nodes, this->velocities, acc, primitives);
        // #pragma omp parallel for collapse(3)
        for(int i = 0; i <= this->param1; i++) {
            for(int j = 0; j <= this->param1; j++) {
                for(int k = 0; k <= this->param1; k++) {
                    int ind = getInd(i, j, k);
                    F3pos[ind] = tmpVels[ind] * dt;
                    F3vel[ind] = acc[ind] * dt;

                    tmpPos[ind] = this->nodes[ind] + F3pos[ind];
                    tmpVels[ind] = this->velocities[ind] + F3vel[ind];
                }
            }
        }

        this->computeAcceleration(this->nodes, this->velocities, acc, primitives);
        // #pragma omp parallel for collapse(3)
        for(int i = 0; i <= this->param1; i++) {
            for(int j = 0; j <= this->param1; j++) {
                for(int k = 0; k <= this->param1; k++) {
                    int ind = getInd(i, j, k);
                    F4pos[ind] = tmpVels[ind] * dt;
                    F4vel[ind] = acc[ind] * dt;

                    this->nodes[ind] += (F1pos[ind] + 2.0 * F2pos[ind] + 2.0 * F3pos[ind] + F4pos[ind]) / 6.0;
                    this->nodes[ind] = glm::min(this->nodes[ind], glm::vec<3, double>(this->maxPos));
                    this->nodes[ind] = glm::max(this->nodes[ind], glm::vec<3, double>(-this->maxPos));
                    this->velocities[ind] += (F1vel[ind] + 2.0 * F2vel[ind] + 2.0 * F3vel[ind] + F4vel[ind]) / 6.0;
                }
            }
        }
    }

    // Calculate new vertex data
    this->calcVertexData();
    // Associate new data with VBO
    glErrorCheck(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
    glErrorCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * this->vertexData.size(), this->vertexData.data(), GL_STATIC_DRAW));
    glErrorCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void JelloCube::scatter() {
    std::uniform_real_distribution<double> sideDis(-20.0, 20.0);
    std::uniform_real_distribution<double> upDis(0, 30.0);
    glm::vec<3, double> velChange(sideDis(this->gen), upDis(this->gen), sideDis(this->gen));
    for(glm::vec<3, double>& vel : this->velocities) {
        vel += velChange;
    }
}


const void JelloCube::calcVertexData() {
    this->vertexData.clear();
    // +x face
    for(int j = 0; j < this->param1; j++) {
        for(int k = 0; k < this->param1; k++) {
            std::vector<std::pair<int, int>> inds = {{j+1, k+1}, {j+1, k}, {j, k+1}, {j, k}};
            std::vector<glm::vec3> vertices(inds.size());
            std::transform(inds.begin(), inds.end(), vertices.begin(), [this](std::pair<int, int> ind) -> glm::vec3 {
                return nodes[this->getInd(this->param1, ind.first, ind.second)];
            });
            glm::vec3 normal = glm::cross(vertices[3] - vertices[2], vertices[0] - vertices[2]);
            std::vector<glm::vec3> normals = {normal, normal, normal, normal};
            std::vector<glm::vec2> uvs(inds.size());
            std::transform(inds.begin(), inds.end(), uvs.begin(), [this](std::pair<int, int> ind) -> glm::vec2 {
                return glm::vec2(1 - (ind.first * this->restLen), ind.second * this->restLen);
            });
            this->makeTile(vertices, normals, uvs, false);
        }
    }

    // -x face
    for(int j = 0; j < this->param1; j++) {
        for(int k = 0; k < this->param1; k++) {
            std::vector<std::pair<int, int>> inds = {{j+1, k}, {j+1, k+1}, {j, k}, {j, k+1}};
            std::vector<glm::vec3> vertices(inds.size());
            std::transform(inds.begin(), inds.end(), vertices.begin(), [this](std::pair<int, int> ind) -> glm::vec3 {
                return nodes[this->getInd(0, ind.first, ind.second)];
            });
            glm::vec3 normal = glm::cross(vertices[3] - vertices[2], vertices[0] - vertices[2]);
            std::vector<glm::vec3> normals = {normal, normal, normal, normal};
            std::vector<glm::vec2> uvs(inds.size());
            std::transform(inds.begin(), inds.end(), uvs.begin(), [this](std::pair<int, int> ind) -> glm::vec2 {
                return glm::vec2(ind.first * this->restLen, ind.second * this->restLen);
            });
            this->makeTile(vertices, normals, uvs, false);
        }
    }

    // +y face
    for(int i = 0; i < this->param1; i++) {
        for(int k = 0; k < this->param1; k++) {
            std::vector<std::pair<int, int>> inds = {{i+1, k+1}, {i, k+1}, {i+1, k}, {i, k}};
            std::vector<glm::vec3> vertices(inds.size());
            std::transform(inds.begin(), inds.end(), vertices.begin(), [this](std::pair<int, int> ind) -> glm::vec3 {
                return nodes[this->getInd(ind.first, this->param1, ind.second)];
            });
            glm::vec3 normal = glm::cross(vertices[3] - vertices[2], vertices[0] - vertices[2]);
            std::vector<glm::vec3> normals = {normal, normal, normal, normal};
            std::vector<glm::vec2> uvs(inds.size());
            std::transform(inds.begin(), inds.end(), uvs.begin(), [this](std::pair<int, int> ind) -> glm::vec2 {
                return glm::vec2(ind.first * this->restLen, 1 - (ind.second * this->restLen));
            });
            this->makeTile(vertices, normals, uvs, false);
        }
    }

    // -y face
    for(int i = 0; i < this->param1; i++) {
        for(int k = 0; k < this->param1; k++) {
            std::vector<std::pair<int, int>> inds = {{i, k+1}, {i+1, k+1}, {i, k}, {i+1, k}};
            std::vector<glm::vec3> vertices(inds.size());
            std::transform(inds.begin(), inds.end(), vertices.begin(), [this](std::pair<int, int> ind) -> glm::vec3 {
                return nodes[this->getInd(ind.first, 0, ind.second)];
            });
            glm::vec3 normal = glm::cross(vertices[3] - vertices[2], vertices[0] - vertices[2]);
            std::vector<glm::vec3> normals = {normal, normal, normal, normal};
            std::vector<glm::vec2> uvs(inds.size());
            std::transform(inds.begin(), inds.end(), uvs.begin(), [this](std::pair<int, int> ind) -> glm::vec2 {
                return glm::vec2(ind.first * this->restLen, ind.second * this->restLen);
            });
            this->makeTile(vertices, normals, uvs, false);
        }
    }

    // +z face
    for(int i = 0; i < this->param1; i++) {
        for(int j = 0; j < this->param1; j++) {
            std::vector<std::pair<int, int>> inds = {{i, j+1}, {i+1, j+1}, {i, j}, {i+1, j}};
            std::vector<glm::vec3> vertices(inds.size());
            std::transform(inds.begin(), inds.end(), vertices.begin(), [this](std::pair<int, int> ind) -> glm::vec3 {
                return nodes[this->getInd(ind.first, ind.second, this->param1)];
            });
            glm::vec3 normal = glm::cross(vertices[3] - vertices[2], vertices[0] - vertices[2]);
            std::vector<glm::vec3> normals = {normal, normal, normal, normal};
            std::vector<glm::vec2> uvs(inds.size());
            std::transform(inds.begin(), inds.end(), uvs.begin(), [this](std::pair<int, int> ind) -> glm::vec2 {
                return glm::vec2(ind.first * this->restLen, ind.second * this->restLen);
            });
            this->makeTile(vertices, normals, uvs, false);
        }
    }

    // -z face
    for(int i = 0; i < this->param1; i++) {
        for(int j = 0; j < this->param1; j++) {
            std::vector<std::pair<int, int>> inds = {{i+1, j+1}, {i, j+1}, {i+1, j}, {i, j}};
            std::vector<glm::vec3> vertices(inds.size());
            std::transform(inds.begin(), inds.end(), vertices.begin(), [this](std::pair<int, int> ind) -> glm::vec3 {
                return nodes[this->getInd(ind.first, ind.second, 0)];
            });
            glm::vec3 normal = glm::cross(vertices[3] - vertices[2], vertices[0] - vertices[2]);
            std::vector<glm::vec3> normals = {normal, normal, normal, normal};
            std::vector<glm::vec2> uvs(inds.size());
            std::transform(inds.begin(), inds.end(), uvs.begin(), [this](std::pair<int, int> ind) -> glm::vec2 {
                return glm::vec2(1 - (ind.first * this->restLen), ind.second * this->restLen);
            });
            this->makeTile(vertices, normals, uvs, false);
        }
    }
}
