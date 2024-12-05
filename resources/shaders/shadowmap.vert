#version 330 core

layout(location = 0) in vec3 objPos;

uniform mat4 modelMat;
uniform mat4 lightSpaceMat;

void main() {
    gl_Position = lightSpaceMat * modelMat * vec4(objPos, 1.0);
}
