#include "camera.h"
#include <cmath>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

const void Camera::updateCamera(const SceneCameraData& cameraData) {
    bool viewChanged = this->pos != cameraData.pos ||
                       this->look != cameraData.look || this->up != cameraData.up;
    this->pos = cameraData.pos;
    this->look = normalize(cameraData.look);
    this->up = normalize(cameraData.up);
    this->right = glm::vec4(normalize(cross(glm::vec3(this->look), glm::vec3(this->up))), 0);

    // Construct view matrix if needed
    if(viewChanged) {
        calcViewMat();
    }

    bool projChanged = this->heightAngle != cameraData.heightAngle;
    this->heightAngle = cameraData.heightAngle;
    if(projChanged) {
        calcProjMat();
    }
}

const void Camera::updateAspectRatio(float aspectRatio) {
    bool projChanged = this->aspectRatio != aspectRatio;
    this->aspectRatio = aspectRatio;
    if(projChanged) {
        calcProjMat();
    }
}

const void Camera::updatePlanes(float nearPlane, float farPlane) {
    bool projChanged = this->nearPlane != nearPlane || this->farPlane != farPlane;
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;
    if(projChanged) {
        calcProjMat();
    }
}

const void Camera::updatePos(glm::vec4 pos) {
    bool viewChanged = this->pos != pos;
    // std::cout << "old position: " << glm::to_string(this->pos) << ", new position: " << glm::to_string(pos) << std::endl;
    this->pos = pos;
    if(viewChanged) {
        calcViewMat();
    }
}

const void Camera::rotateCamera(glm::vec4 axis, float angle) {
    glm::vec4 u = normalize(axis);
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    glm::mat4 rotMatrix(
        cosTheta + u.x * (1 - cosTheta), u.x * u.y * (1 - cosTheta) + u.z * sinTheta, u.x * u.z * (1 - cosTheta) - u.y * sinTheta, 0,
        u.x * u.y * (1 - cosTheta) - u.z * sinTheta, cosTheta + u.y * u.y * (1 - cosTheta), u.y * u.z * (1 - cosTheta) + u.x * sinTheta, 0,
        u.x * u.z * (1 - cosTheta) + u.y * sinTheta, u.y * u.z * (1 - cosTheta) - u.x * sinTheta, cosTheta + u.z * u.z * (1 - cosTheta), 0,
        0, 0, 0, 1
    );
    this->look = rotMatrix * this->look;
    this->up = rotMatrix * this->up;
    this->right = glm::vec4(normalize(cross(glm::vec3(this->look), glm::vec3(this->up))), 0);
    calcViewMat();
}

void Camera::calcViewMat() {
    glm::mat4 cameraTrans = glm::mat4(1.0);
    cameraTrans[3][0] = -this->pos.x;
    cameraTrans[3][1] = -this->pos.y;
    cameraTrans[3][2] = -this->pos.z;

    glm::vec4 w = -glm::normalize(this->look);
    glm::vec4 v = glm::normalize(this->up - glm::dot(this->up, w) * w);
    glm::vec4 u = glm::vec4(glm::cross(glm::vec3(v), glm::vec3(w)), 0);
    glm::mat4 cameraRot = glm::transpose(glm::mat4(
        u, v, w,
        glm::vec4(0, 0, 0, 1)
    ));
    this->viewMat = cameraRot * cameraTrans;
}

void Camera::calcProjMat() {
    float farHalfHeight = farPlane * tan(heightAngle / 2);
    float farHalfWidth = farHalfHeight * this->aspectRatio;
    glm::mat4 scaleMat(
        1 / farHalfWidth, 0, 0, 0,
        0, 1 / farHalfHeight, 0, 0,
        0, 0, 1 / farPlane, 0,
        0, 0, 0, 1
    );
    float c = -nearPlane / farPlane;
    glm::mat4 unhingeMat(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1 / (1 + c), -1,
        0, 0, -c / (1 + c), 0
    );
    glm::mat4 adjustMat(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -2, 0,
        0, 0, -1, 1
    );
    this->projMat = adjustMat * unhingeMat * scaleMat;
}

const glm::mat4& Camera::getViewMatrix() const {
    return this->viewMat;
}

const glm::mat4& Camera::getProjMatrix() const {
    return this->projMat;
}

const glm::vec4& Camera::getPosition() const {
    return this->pos;
}

const glm::vec4& Camera::getLook() const {
    return this->look;
}

const glm::vec4& Camera::getUp() const {
    return this->up;
}

const glm::vec4& Camera::getRight() const {
    return this->right;
}

const float Camera::getAspectRatio() const {
    return this->aspectRatio;
}
