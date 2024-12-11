#ifndef REALTIMESCENE_H
#define REALTIMESCENE_H

#include <string>
#include <vector>
#include <memory>
#include <random>

#include "GL/glew.h"
#include "camera.h"
#include "primitives.h"
#include "settings.h"

class RealtimeScene {
public:
    RealtimeScene();

    void initScene();
    void resetScene();
    void free() {
        this->primitives.clear();
        this->lights.clear();
    }

    // Binds uniforms related to the scene
    void bindSceneUniforms(GLuint shader);

    // The getter of the scene's primitives
    std::vector<std::unique_ptr<Primitive>>& getPrimitives();

    void updateScene();
    void scatterCube();
    void addObstacle();

    // The getter of the shared pointer to the camera instance of the scene
    Camera& getCamera();
private:
    std::string scenefile;

    Camera camera;
    SceneGlobalData globalData;

    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<SceneLightData> lights;

    SceneMaterial jelloMaterial = {
        .cAmbient = glm::vec4(0.2, 0.8, 0.2, 1),
        .cDiffuse = glm::vec4(0.2, 0.8, 0.2, 1),
        .cSpecular = glm::vec4(1, 1, 1, 1),
        .shininess = 25
    };
    SceneMaterial& getJelloMaterial() {
        this->jelloMaterial.cDiffuse.a = settings.transparentCube ? 0.5 : 1;
        return this->jelloMaterial;
    }
    SceneMaterial obstacleMaterial = {
        .cAmbient = glm::vec4(0, 0, 0, 1),
        .cDiffuse = glm::vec4(0.3, 0.3, 0.3, 1),
        .cSpecular = glm::vec4(1, 1, 1, 1),
        .shininess = 30,
        .textureMap = SceneFileMap{
            .isUsed = true,
            .filename = "textures/liqmtl.png",
            .repeatU = 1,
            .repeatV = 1,
        },
        .blend = 0.4
    };


    // For object generation
    std::mt19937 gen;
    float randFloat(float min, float max);
};

#endif // REALTIMESCENE_H
