#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

layout(location = 0) in VertData
{
    vec3 normal;
    vec2 fragTexCoord;
} inVertData;

layout(location = 2) in LightingData
{
    int count;
    vec4 spaces[LIGHT_MAX_COUNT];
    vec3 dirs[LIGHT_MAX_COUNT];
} inLightingData;

layout (set = 1, binding = 1) uniform sampler2D shadowMaps[LIGHT_MAX_COUNT];
layout (set = 2, binding = 0) uniform sampler2D diffuseSampler;

layout(location = 0) out vec4 outColor;

float SampleShadowMap(vec2 projCoords, float currentDepth)
{
    float closestDepth = texture(shadowMaps[0], projCoords.xy).r; 
    return currentDepth > closestDepth ? 1 : 0;
}

float CalcPCF(vec3 projCoords, float currentDepth)
{
    float shadow = 0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[0], 0);

    for(int x = -1; x <= 1; ++x)
        for(int y = -1; y <= 1; ++y)
            shadow += SampleShadowMap(projCoords.xy + vec2(x, y) * texelSize, currentDepth);

    shadow /= 9;
    return shadow;
}

float CalculateShadow()
{
    float bias = 0.005;
    vec3 projCoords = inLightingData.spaces[0].xyz / inLightingData.spaces[0].w;
    float currentDepth = projCoords.z - bias;
    if(currentDepth <= 0.0 || currentDepth >= 1.0)
        return 1;

    projCoords = projCoords * 0.5 + 0.5;
    return CalcPCF(projCoords, currentDepth);
}

void main() 
{
    outColor = texture(diffuseSampler, inVertData.fragTexCoord) * CalculateShadow();
}