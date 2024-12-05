#version 330 core

in vec2 uvCoords;

uniform sampler2D outputXImage;
uniform sampler2D outputYImage;

out vec4 fragColor;

void main()
{
    vec4 xColor = texture(outputXImage, uvCoords);
    vec4 yColor = texture(outputYImage, uvCoords);
    fragColor = clamp(0.25 * sqrt(xColor * xColor + yColor * yColor), 0, 1);
}
