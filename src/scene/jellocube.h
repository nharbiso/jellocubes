#ifndef JELLOCUBE_H
#define JELLOCUBE_H

#include "primitives.h"
#include <random>

class JelloCube : public Cube {
public:
    JelloCube(const SceneMaterial& material, int param, glm::vec<3, double> center);

    void update();
    void scatter();

    const void calcVertexData() override;
private:
    double restLen; // resting length between two adjacent nodes
    std::vector<glm::vec<3, double>> nodes; // contains param^3 nodes, which internally interact
    std::vector<glm::vec<3, double>> velocities; // velocities of each node
    inline int getInd(int i, int j, int k) {
        return i * (this->param + 1) * (this->param + 1) + j * (this->param + 1) + k;
    }

    // Computes hooks force on node1, due to spring between node1 and node2 (given by their indices)
    inline glm::vec<3, double> hooksForce(int ind1, int ind2, double k, double restLen) {
        return hooksForce(this->nodes[ind1], this->nodes[ind2], k, restLen);
    }
    // Computes hooks force on a node, due to spring between it and another point
    inline glm::vec<3, double> hooksForce(glm::vec<3, double>& pos1, glm::vec<3, double>& pos2, double k, double restLen) {
        glm::vec<3, double> posDiff = pos1 - pos2;
        // std::cout << "length: " << glm::length(posDiff) << ", resting length: " << restLen << std::endl;
        return (-k * (glm::length(posDiff) - restLen)) * normalize(posDiff);
    }

    // Computes dampening force on node1, due to spring between node1 and node2
    inline glm::vec<3, double> dampeningForce(int ind1, int ind2, double k) {
        return dampeningForce(this->nodes[ind1], this->nodes[ind2], this->velocities[ind1], this->velocities[ind2], k);
    }
    // Computes dampening force on a node, due to spring between it and another point
    inline glm::vec<3, double> dampeningForce(glm::vec<3, double>& pos1, glm::vec<3, double>& pos2, glm::vec<3, double>& vel1, glm::vec<3, double>& vel2, double k) {
        glm::vec<3, double> posDiff = pos1 - pos2;
        double len = glm::length(posDiff);
        return (-k * glm::dot(vel1 - vel2, posDiff) / (len * len)) * posDiff;
    }

    glm::vec<3, double> getStructuralForce(int i, int j, int k);
    glm::vec<3, double> getShearForce(int i, int j, int k);
    glm::vec<3, double> getBendForce(int i, int j, int k);
    glm::vec<3, double> getCollisionForce(int i, int j, int k);
    void computeAcceleration(std::vector<glm::vec<3, double>>& nodes,
                             std::vector<glm::vec<3, double>>& velocities,
                             std::vector<glm::vec<3, double>>& acc);

    // For scattering
    std::mt19937 gen;
    std::uniform_real_distribution<double> sideDis;
    std::uniform_real_distribution<double> upDis;
};

#endif // JELLOCUBE_H
