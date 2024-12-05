#version 330 core

in vec2 uvCoords;

uniform sampler2D outputImage;
uniform int screenWidth;
uniform int screenHeight;

uniform float convMat[25];
uniform int convDim;

out vec4 fragColor;

void main()
{
    fragColor = vec4(0, 0, 0, 1);
    vec2 xOffset = vec2(1.0f / screenWidth, 0);
    vec2 yOffset = vec2(0, 1.0f / screenHeight);
    int mid = convDim / 2;
    for(int i = 0; i < convDim; i++) {
        for(int j = 0; j < convDim; j++) {
            fragColor += convMat[j * convDim + i] * texture(outputImage, uvCoords + (i-mid) * xOffset + (mid-j) * yOffset);
        }
    }
}
