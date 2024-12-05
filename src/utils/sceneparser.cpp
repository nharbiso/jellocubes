#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    renderData.shapes.clear();
    renderData.lights.clear();
    parseSceneGraph(fileReader.getRootNode(), glm::mat4(1.0), renderData.shapes, renderData.lights);

    return true;
}

void SceneParser::parseSceneGraph(SceneNode* node, glm::mat4 ctm,
                                  std::vector<RenderShapeData>& shape_data,
                                  std::vector<SceneLightData>& light_data) {
    ctm *= constructTransMat(node->transformations);
    for(ScenePrimitive* primitive : node->primitives) {
        shape_data.push_back(RenderShapeData{
            .primitive = *primitive,
            .ctm = ctm
        });
    }
    for(SceneLight* light : node->lights) {
        light_data.push_back(SceneLightData{
            .id = light->id,
            .type = light->type,
            .color = light->color,
            .function = light->function,
            .pos = ctm * glm::vec4(0, 0, 0, 1),
            .dir = ctm * light->dir,
            .penumbra = light->penumbra,
            .angle = light->angle,
            .width = light->width,
            .height = light->height
        });
    }
    for(SceneNode* child : node->children) {
        parseSceneGraph(child, ctm, shape_data, light_data);
    }
}

glm::mat4 SceneParser::constructTransMat(std::vector<SceneTransformation*>& transforms) {
    glm::mat4 transMat = glm::mat4(1.0);
    for(SceneTransformation* transform : transforms) {
        if(transform->type == TransformationType::TRANSFORMATION_TRANSLATE) {
            transMat *= glm::translate(transform->translate);
        } else if(transform->type == TransformationType::TRANSFORMATION_ROTATE) {
            transMat *= glm::rotate(transform->angle, transform->rotate);
        } else if(transform->type == TransformationType::TRANSFORMATION_SCALE) {
            transMat *= glm::scale(transform->scale);
        } else {
            transMat *= transform->matrix;
        }
    }
    return transMat;
}
