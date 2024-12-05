#version 330 core

layout(location = 0) in vec3 objPos;
layout(location = 1) in vec3 objNormal;
layout(location = 2) in vec2 inUVCoords;

out vec4 worldPos;
out vec4 worldNormal;
out vec2 uvCoords;

uniform mat4 modelMat;
uniform mat3 normalMat;
uniform mat4 viewMat;
uniform mat4 projMat;

void main() {
    worldPos = modelMat * vec4(objPos, 1.0);
    worldNormal = vec4(normalMat * objNormal, 0);
    uvCoords = inUVCoords;

    gl_Position = projMat * viewMat * worldPos;
}
