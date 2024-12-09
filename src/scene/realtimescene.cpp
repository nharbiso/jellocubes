#include "realtimescene.h"

#include "utils/sceneparser.h"
#include "settings.h"
#include "jellocube.h"
#include <unordered_map>

std::unordered_map<std::string, QImage> fileToTexture;

RealtimeScene::RealtimeScene() {
    this->scenefile = "";
    this->camera = Camera();
}

void RealtimeScene::initScene(GLuint shadowMapShader) {
    RenderData renderData;
    SceneParser::parse("scenefile.json", renderData);

    fileToTexture.clear();
    for(const RenderShapeData& shapeData : renderData.shapes) {
        const SceneFileMap& textureMap = shapeData.primitive.material.textureMap;
        if(textureMap.isUsed && !fileToTexture.contains(textureMap.filename)) {
            fileToTexture[textureMap.filename] = QImage(textureMap.filename.c_str()).convertToFormat(QImage::Format_RGBA8888).mirrored();
        }
    }

    RenderShapeData& shapeData = renderData.shapes[0];
    bounds = 4;
    std::unique_ptr<Primitive> boundingBox = std::make_unique<Cube>(
        glm::scale(shapeData.ctm, glm::vec3(2 * bounds)),
        shapeData.primitive.material,
        8, true
    );
    boundingBox->initialize();
    this->primitives.push_back(std::move(boundingBox));

    SceneMaterial jelloMaterial = {
        .cAmbient = glm::vec4(0.2, 0.8, 0.2, 1),
        .cDiffuse = glm::vec4(0.2, 0.8, 0.2, 0.5),
        .cSpecular = glm::vec4(0.2, 0.8, 0.2, 1),
        .shininess = 10
    };
    std::unique_ptr<Primitive> jelloCube = std::make_unique<JelloCube>(jelloMaterial, 8, glm::vec3(0, 0, 0));
    jelloCube->initialize();
    this->primitives.push_back(std::move(jelloCube));

    this->camera.updateCamera(renderData.cameraData);
    this->globalData = renderData.globalData;

    this->lights.clear(); // remove previous lights and free any underlying memory
    for(int i = 0; i < renderData.lights.size(); i++) {
        this->lights.push_back(Light(renderData.lights[i], this->primitives, shadowMapShader, i+1));
    }
}

void RealtimeScene::updateScene() {
    for(int i = 1; i < this->primitives.size(); i++) {
        JelloCube* jelloCube = static_cast<JelloCube*>(this->primitives[i].get());
        jelloCube->update();
    }
}

void RealtimeScene::bindSceneUniforms(GLuint shader) {
    // Camera data
    glErrorCheck(glUniformMatrix4fv(glGetUniformLocation(shader, "viewMat"), 1, false, &this->camera.getViewMatrix()[0][0]));
    glErrorCheck(glUniformMatrix4fv(glGetUniformLocation(shader, "projMat"), 1, false, &this->camera.getProjMatrix()[0][0]));
    glErrorCheck(glUniform4fv(glGetUniformLocation(shader, "cameraPos"), 1, &this->camera.getPosition()[0]));

    // Global data
    glErrorCheck(glUniform1f(glGetUniformLocation(shader, "ka"), this->globalData.ka));
    glErrorCheck(glUniform1f(glGetUniformLocation(shader, "kd"), this->globalData.kd));
    glErrorCheck(glUniform1f(glGetUniformLocation(shader, "ks"), this->globalData.ks));

    // Light data
    int i = 0;
    for(Light& light : this->lights) {
        std::string entry = "lights[" + std::to_string(i) + "]";
        glErrorCheck(glUniform1i(
            glGetUniformLocation(shader, (entry + ".type").c_str()),
            (GLint) light.lightData.type
        ));
        glErrorCheck(glUniform4fv(
            glGetUniformLocation(shader, (entry + ".color").c_str()),
            1, &light.lightData.color[0]
        ));
        glErrorCheck(glUniform3fv(
            glGetUniformLocation(shader, (entry + ".function").c_str()),
            1, &light.lightData.function[0]
        ));
        glErrorCheck(glUniform4fv(
            glGetUniformLocation(shader, (entry + ".pos").c_str()),
            1, &light.lightData.pos[0]
        ));
        glErrorCheck(glUniform4fv(
            glGetUniformLocation(shader, (entry + ".dir").c_str()),
            1, &light.lightData.dir[0]
        ));
        glErrorCheck(glUniform1f(
            glGetUniformLocation(shader, (entry + ".penumbra").c_str()),
            light.lightData.penumbra
        ));
        glErrorCheck(glUniform1f(
            glGetUniformLocation(shader, (entry + ".angle").c_str()),
            light.lightData.angle
        ));
        // glErrorCheck(glUniform1i(
        //     glGetUniformLocation(shader, (entry + ".shadowMap").c_str()),
        //     i + 1
        // ));
        // glErrorCheck(glActiveTexture(GL_TEXTURE0 + i + 1));
        // glErrorCheck(glBindTexture(GL_TEXTURE_2D, light.shadowMap));
        // glErrorCheck(glUniformMatrix4fv(
        //     glGetUniformLocation(shader, (entry + ".lightSpaceMat").c_str()),
        //     1, false, &light.lightCamera.lightSpaceMatrix[0][0]
        // ));
        i++;

        if(i == 8) {
            break;
        }
    }
    glErrorCheck(glUniform1i(glGetUniformLocation(shader, "numLights"), i));
}

std::vector<std::unique_ptr<Primitive>>& RealtimeScene::getPrimitives() {
    return this->primitives;
}

Camera& RealtimeScene::getCamera() {
    return this->camera;
}


Light::Light(const SceneLightData& lightData,
             std::vector<std::unique_ptr<Primitive>>& primitives,
             GLuint shadowMapShader, int textureInd) : lightCamera(lightData, primitives) {
    this->lightData = lightData;
    int viewWidth = round(this->RESOLUTION * this->lightCamera.getAspectRatio());
    int viewHeight = this->RESOLUTION;

    // Generate shadow map
    glErrorCheck(glGenTextures(1, &this->shadowMap));

    glErrorCheck(glActiveTexture(GL_TEXTURE0 + textureInd));
    glErrorCheck(glBindTexture(GL_TEXTURE_2D, this->shadowMap));
    glErrorCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, viewWidth, viewHeight, 0,
                              GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
    glErrorCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    glErrorCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    glErrorCheck(glBindTexture(GL_TEXTURE_2D, 0));

    // Generate and bind FBO
    glErrorCheck(glGenFramebuffers(1, &this->shadowFBO));
    glErrorCheck(glBindFramebuffer(GL_FRAMEBUFFER, this->shadowFBO));

    // Add texture as a depth attachment to FBO
    glErrorCheck(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->shadowMap, 0));
    glErrorCheck(glDrawBuffer(GL_NONE));
    glErrorCheck(glReadBuffer(GL_NONE));

    // Render scene from light's POV as camera to FBO
    // Clear screen color and depth and adjust viewport before rendering
    glErrorCheck(glViewport(0, 0, viewWidth, viewHeight));
    glErrorCheck(glClear(GL_DEPTH_BUFFER_BIT));

    // Activate shader program
    glErrorCheck(glUseProgram(shadowMapShader));

    glErrorCheck(glUniformMatrix4fv(glGetUniformLocation(shadowMapShader, "lightSpaceMat"), 1, false, &this->lightCamera.lightSpaceMatrix[0][0]));
    for(const std::unique_ptr<Primitive>& primitive : primitives)
        primitive->draw(shadowMapShader, true);

    // Deactivate shader program
    glErrorCheck(glUseProgram(0));

    // Unbind the FBO
    glErrorCheck(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}
