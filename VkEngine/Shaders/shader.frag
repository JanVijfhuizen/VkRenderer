#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

layout(location = 0) in Data
{
    vec3 normal;
    vec2 fragTexCoord;
    flat int count;
    vec4 spaces[LIGHT_MAX_COUNT];
    vec3 dirs[LIGHT_MAX_COUNT];
} inData;

layout (set = 1, binding = 1) uniform sampler2D shadowMaps[LIGHT_MAX_COUNT];
layout (set = 2, binding = 0) uniform sampler2D diffuseSampler;

layout(location = 0) out vec4 outColor;

float SampleShadowMap(int index, vec2 projCoords, float currentDepth)
{
    float closestDepth = texture(shadowMaps[index], projCoords.xy).r; 
    return currentDepth > closestDepth ? 1 : 0;
}

float CalcPCF(int index, vec3 projCoords, float currentDepth)
{
    float shadow = 0;
    vec2 texelSize = 1.0 / textureSize(shadowMaps[index], 0);

    for(int x = -1; x <= 1; ++x)
        for(int y = -1; y <= 1; ++y)
            shadow += SampleShadowMap(index, projCoords.xy + vec2(x, y) * texelSize, currentDepth);

    shadow /= 9;
    return shadow;
}

float CalculateShadow(int index)
{
    float bias = 0.005;
    vec3 projCoords = inData.spaces[index].xyz / inData.spaces[index].w;
    float currentDepth = projCoords.z - bias;
    if(currentDepth <= 0.0 || currentDepth >= 1.0)
        return 1;

    projCoords = projCoords * 0.5 + 0.5;
    return CalcPCF(index, projCoords, currentDepth);
}

void main() 
{
    vec4 diffuse = texture(diffuseSampler, inData.fragTexCoord);

    float shadow = 0;
    for(int i = 0; i < 1; i++)
        shadow += CalculateShadow(i);
    shadow = min(1, shadow);

    outColor = diffuse * shadow;
}