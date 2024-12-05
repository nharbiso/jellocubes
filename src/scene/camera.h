#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "utils/scenedata.h"

class Camera {
public:
    // Updates the camera and its internal metadata by the given camera metadata
    const void updateCamera(const SceneCameraData& data);

    // Updates the camera and its internal metadata by the given window aspect ratio
    const void updateAspectRatio(float aspectRatio);

    // Updates the camera and its internal metadata by the given near and far plane
    const void updatePlanes(float nearPlane, float farPlane);

    // Updates the camera and its internal metadata by the given new position
    const void updatePos(glm::vec4 pos);

    // Rotates the camera around the given axis CCW by the given angle in radians
    const void rotateCamera(glm::vec4 axis, float angle);

    // Returns the view matrix for the current camera settings.
    const glm::mat4& getViewMatrix() const;

    // Returns the projection matrix for the current camera settings.
    const glm::mat4& getProjMatrix() const;

    // Returns the position of the camera in the world space.
    const glm::vec4& getPosition() const;

    // Returns the unit look vector of the camera in world space.
    const glm::vec4& getLook() const;

    // Returns the unit up vector of the camera in world space.
    const glm::vec4& getUp() const;

    // Returns the unit vector pointing to the right relative to the look
    // and up vector of the camera in world space.
    const glm::vec4& getRight() const;

    // Returns the aspect ratio of the camera
    const float getAspectRatio() const;

protected:
    glm::vec4 pos;
    glm::vec4 look;
    glm::vec4 up;
    glm::vec4 right;

    glm::mat4 viewMat;
    void calcViewMat();
    glm::mat4 projMat;
    void calcProjMat();

    float aspectRatio;
    float heightAngle;
    float nearPlane, farPlane;
};

#endif // CAMERA_H
