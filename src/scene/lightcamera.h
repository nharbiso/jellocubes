#ifndef LIGHTCAMERA_H
#define LIGHTCAMERA_H

#include "utils/scenedata.h"
#include "camera.h"
#include "primitives.h"

class LightCamera : public Camera {
public:
    LightCamera(const SceneLightData& lightData, std::vector<std::unique_ptr<Primitive>>& primitives) {
        if(lightData.type == LightType::LIGHT_SPOT) {
            SceneCameraData cameraData = {
                .pos = lightData.pos,
                .look = lightData.dir,
                .up = glm::vec4(0, 1, 0, 0),
                .heightAngle = lightData.angle
            };
            this->updateCamera(cameraData);
        }
        this->aspectRatio = 1;
        this->lightSpaceMatrix = this->projMat * this->viewMat;
    }

    glm::mat4 lightSpaceMatrix;
};

#endif // LIGHTCAMERA_H
