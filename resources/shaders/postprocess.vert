#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 inUVCoords;

out vec2 uvCoords;

void main() {
    uvCoords = inUVCoords;

    gl_Position = vec4(position, 1.0);
}
