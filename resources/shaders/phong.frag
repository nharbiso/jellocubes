#version 330 core

in vec4 worldPos;
in vec4 worldNormal;
in vec2 uvCoords;

out vec4 fragColor;

// Global data
uniform float ka;
uniform float kd;
uniform float ks;

// Material data
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform vec4 materialSpecular;
uniform float materialShininess;
uniform bool textured;
uniform sampler2D texture;
uniform float materialBlend;

uniform bool textureMapEnabled;

// Light data
const int LIGHT_POINT = 0;
const int LIGHT_DIRECTIONAL = 1;
const int LIGHT_SPOT = 2;
struct LightData {
    int type;
    vec4 color;

    vec3 function;
    vec4 pos;
    vec4 dir;

    float penumbra;
    float angle;

    sampler2D shadowMap;
    mat4 lightSpaceMat;
};
uniform LightData lights[8];
uniform int numLights;
uniform bool shadowMapEnabled;

uniform vec4 cameraPos;

void main() {
    vec4 normalizedNormal = normalize(worldNormal);
    vec4 posToCamera = normalize(cameraPos - worldPos);
    fragColor = vec4(0.0);

    // Add ambient component to output color
    fragColor += ka * materialAmbient;

    for(int i = 0; i < numLights; i++) {
        if(shadowMapEnabled && lights[i].type == LIGHT_SPOT) {
            vec4 lightSpacePos = lights[i].lightSpaceMat * worldPos;
            vec3 projPos = lightSpacePos.xyz / lightSpacePos.w;
            projPos = projPos * 0.5 + 0.5; // normalize from [-1, 1] to [0, 1]
            float shadowMapDepth = texture(lights[i].shadowMap, projPos.xy).x;
            if(projPos.z > shadowMapDepth) {
                continue; // projection depth greater, therefore in shadow
            }
        }

        vec4 posToLight;
        float f_att = 1; // attenuation
        // Calculate attenuation and falloff (latter is for spot light)
        if(lights[i].type == LIGHT_DIRECTIONAL) { // directional light
            posToLight = -lights[i].dir;
        } else { // point or spot light
            posToLight = lights[i].pos - worldPos;
            float distToLight = length(posToLight);

            // Calculate attenuation
            vec3 att = lights[i].function;
            f_att = min(1, 1.0 / (att.x + att.y * distToLight + att.z * distToLight * distToLight));

            if(lights[i].type == LIGHT_SPOT) { // adjust attenuation by falloff for spot lights
                float theta = acos(dot(-posToLight, normalize(lights[i].dir)) / distToLight);
                float thetaOuter = lights[i].angle;
                float thetaInner = thetaOuter - lights[i].penumbra;
                if(thetaInner < theta && theta <= thetaOuter) {
                    float thetaNorm = (theta - thetaInner) / (thetaOuter - thetaInner);
                    f_att *= 1 - ((-2 * thetaNorm + 3) * thetaNorm * thetaNorm);
                } else if(theta > thetaOuter) {
                    continue; // point light does not contribute, outside of penumbra
                }
            }
        }

        // Add diffuse component to output color
        vec4 normPosToLight = normalize(posToLight);
        float lightDotNormal = dot(normalizedNormal, normPosToLight);
        if(lightDotNormal > 0) {
            vec4 diffuseColor = kd * materialDiffuse;
            // Blend with texture at intersection point, if applicable
            if(textureMapEnabled && textured) {
                diffuseColor = materialBlend * texture(texture, uvCoords) + (1 - materialBlend) * diffuseColor;
            }
            fragColor += f_att * lights[i].color * diffuseColor * lightDotNormal;
        }

        // Add specular component to output color
        vec4 reflected = normalize(reflect(-normPosToLight, normalizedNormal));
        float cameraDotReflect = dot(reflected, posToCamera);
        if(cameraDotReflect > 0) {
            vec4 specular = f_att * ks * lights[i].color * materialSpecular;
            if(materialShininess > 0) {
                specular *= pow(cameraDotReflect, materialShininess);
            }
            fragColor += specular;
        }
        fragColor.a = materialDiffuse.a;
    }
}
