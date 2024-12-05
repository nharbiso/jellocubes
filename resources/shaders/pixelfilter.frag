#version 330 core

in vec2 uvCoords;

uniform sampler2D outputImage;
uniform int screenWidth;
uniform int screenHeight;

uniform bool grayscale;
uniform bool invert;
uniform bool brighten;

uniform bool sharpen;
uniform bool blur;

out vec4 fragColor;

void main()
{
    fragColor = texture(outputImage, uvCoords);

    if (brighten) {
        fragColor = vec4(clamp(fragColor.r * 1.2, 0, 1), clamp(fragColor.g * 1.2, 0, 1), clamp(fragColor.b * 1.2, 0, 1), 1);
    }
    if (invert) {
        fragColor = vec4(1 - fragColor.r, 1 - fragColor.g, 1 - fragColor.b, 1);
    }
    if (grayscale) {
        float gray = 0.299 * fragColor.r + 0.587 * fragColor.g + 0.114 * fragColor.b;
        fragColor = vec4(gray, gray, gray, 1);
    }
}
