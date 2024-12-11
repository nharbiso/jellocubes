#include "realtimescene.h"

#include "utils/sceneparser.h"
#include "settings.h"
#include "jellocube.h"
#include <unordered_map>
#include <span>

std::unordered_map<std::string, QImage> fileToTexture;

RealtimeScene::RealtimeScene() {
    this->scenefile = "";
    this->camera = Camera();
    this->camera.updatePlanes(settings.nearPlane, settings.farPlane);

    std::random_device rd;
    this->gen = std::mt19937(rd());
}

void RealtimeScene::initScene() {
    RenderData renderData;
    SceneParser::parse("scenefile.json", renderData);

    fileToTexture.clear();
    for(const RenderShapeData& shapeData : renderData.shapes) {
        const SceneFileMap& textureMap = shapeData.primitive.material.textureMap;
        if(textureMap.isUsed && !fileToTexture.contains(textureMap.filename)) {
            fileToTexture[textureMap.filename] = QImage(textureMap.filename.c_str()).convertToFormat(QImage::Format_RGBA8888).mirrored();
        }
    }
    SceneFileMap& obstacleTexture = this->obstacleMaterial.textureMap;
    if(obstacleTexture.isUsed && !fileToTexture.contains(obstacleTexture.filename)) {
        fileToTexture[obstacleTexture.filename] = QImage(obstacleTexture.filename.c_str()).convertToFormat(QImage::Format_RGBA8888).mirrored();
    }

    RenderShapeData& shapeData = renderData.shapes[0];
    settings.bounds = 4;
    std::unique_ptr<Primitive> boundingBox = std::make_unique<Cube>(
        glm::scale(shapeData.ctm, glm::vec3(2 * settings.bounds)),
        shapeData.primitive.material,
        8, true
    );
    boundingBox->initialize();
    this->primitives.push_back(std::move(boundingBox));

    std::unique_ptr<Primitive> jelloCube = std::make_unique<JelloCube>(getJelloMaterial(), 8, glm::vec3(0, 0, 0));
    jelloCube->initialize();
    this->primitives.push_back(std::move(jelloCube));

    this->camera.updateCamera(renderData.cameraData);
    this->globalData = renderData.globalData;

    this->lights.clear(); // remove previous lights and free any underlying memory
    for(SceneLightData& lightData : renderData.lights) {
        this->lights.push_back(lightData);
    }
}

void RealtimeScene::resetScene() {
    this->primitives.erase(this->primitives.begin() + 1, this->primitives.end()); // erase all primitives except bounding box
    std::unique_ptr<Primitive> jelloCube = std::make_unique<JelloCube>(getJelloMaterial(), 8, glm::vec3(0, 0, 0));
    jelloCube->initialize();
    this->primitives.push_back(std::move(jelloCube));
}

void RealtimeScene::updateScene() {
    std::span<std::unique_ptr<Primitive>> interPrimitives(this->primitives.begin() + 1, this->primitives.end() - 1);
    for(int i = 1; i < this->primitives.size(); i++) {
        if (JelloCube* jelloCube = dynamic_cast<JelloCube*>(this->primitives[i].get())) {
            jelloCube->update(interPrimitives);
        }
    }
}

void RealtimeScene::scatterCube() {
    for(int i = 1; i < this->primitives.size(); i++) {
        if (JelloCube* jelloCube = dynamic_cast<JelloCube*>(this->primitives[i].get())) {
            jelloCube->scatter();
        }
    }
}

float RealtimeScene::randFloat(float min, float max) {
    std::uniform_real_distribution<float> dis(min, max);
    return dis(this->gen);
}

void RealtimeScene::addObstacle() {
    glm::vec3 rotAxis(randFloat(-1, 1), randFloat(-1, 1), randFloat(-1, 1));
    float angle = randFloat(0, 2 * M_PI);
    glm::vec3 scale(randFloat(0.5, (float)settings.bounds), randFloat(0.5, (float)settings.bounds), randFloat(0.5, (float)settings.bounds));
    // limit translation to within bounding box
    float maxTranslate = settings.bounds - glm::length(0.5f * scale);
    glm::vec3 translate(randFloat(-maxTranslate, maxTranslate), randFloat(-maxTranslate, maxTranslate), randFloat(-maxTranslate, maxTranslate));
    glm::mat4 ctm = glm::scale(glm::rotate(glm::translate(glm::mat4(1), translate), angle, rotAxis), scale);

    std::unique_ptr<Primitive> primitive;
    if(rand() % 2 == 0)
        primitive = std::make_unique<Cube>(ctm, this->obstacleMaterial, 1, false);
    else
        primitive = std::make_unique<Sphere>(ctm, this->obstacleMaterial, 25, 25);
    primitive->initialize();
    this->primitives.insert(this->primitives.end() - 1, std::move(primitive));
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
    for(SceneLightData lightData : this->lights) {
        std::string entry = "lights[" + std::to_string(i) + "]";
        glErrorCheck(glUniform1i(
            glGetUniformLocation(shader, (entry + ".type").c_str()),
            (GLint) lightData.type
        ));
        glErrorCheck(glUniform4fv(
            glGetUniformLocation(shader, (entry + ".color").c_str()),
            1, &lightData.color[0]
        ));
        glErrorCheck(glUniform3fv(
            glGetUniformLocation(shader, (entry + ".function").c_str()),
            1, &lightData.function[0]
        ));
        glErrorCheck(glUniform4fv(
            glGetUniformLocation(shader, (entry + ".pos").c_str()),
            1, &lightData.pos[0]
        ));
        glErrorCheck(glUniform4fv(
            glGetUniformLocation(shader, (entry + ".dir").c_str()),
            1, &lightData.dir[0]
        ));
        glErrorCheck(glUniform1f(
            glGetUniformLocation(shader, (entry + ".penumbra").c_str()),
            lightData.penumbra
        ));
        glErrorCheck(glUniform1f(
            glGetUniformLocation(shader, (entry + ".angle").c_str()),
            lightData.angle
        ));
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
