#ifndef REALTIMESCENE_H
#define REALTIMESCENE_H

#include <string>
#include <vector>
#include <memory>

#include "GL/glew.h"
#include "camera.h"
#include "lightcamera.h"
#include "primitives.h"
#include "jellocube.h"

// Representation of a light and its shadow map
class Light {
public:
    Light(const SceneLightData& lightData,
          std::vector<std::unique_ptr<Primitive>>& primitives,
          GLuint shadowMapShader, int textureInd);

    void initialize();

    ~Light() {
        glErrorCheck(glDeleteTextures(1, &this->shadowMap));
        glErrorCheck(glDeleteFramebuffers(1, &this->shadowFBO));
    }

    SceneLightData lightData;
    LightCamera lightCamera;
    GLuint shadowFBO;
    GLuint shadowMap;

    const static int RESOLUTION = 1024;
};

class RealtimeScene {
public:
    RealtimeScene();

    void initScene(GLuint shadowMapShader);
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

    // The getter of the shared pointer to the camera instance of the scene
    Camera& getCamera();
private:
    std::string scenefile;

    Camera camera;
    SceneGlobalData globalData;

    std::vector<std::unique_ptr<Primitive>> primitives;

    std::vector<Light> lights;
};

#endif // REALTIMESCENE_H
