#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inFragTexCoord;
layout(location = 2) in vec4 lightSpace;

layout (set = 1, binding = 1) uniform sampler2D shadowMap;
layout (set = 2, binding = 0) uniform sampler2D diffuseSampler;

layout(location = 0) out vec4 outColor;

float SampleShadowMap(vec2 projCoords, float currentDepth)
{
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    return currentDepth > closestDepth ? 1 : 0;
}

float CalcPCF(vec3 projCoords, float currentDepth)
{
    float shadow = 0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int x = -1; x <= 1; ++x)
        for(int y = -1; y <= 1; ++y)
            shadow += SampleShadowMap(projCoords.xy + vec2(x, y) * texelSize, currentDepth);

    shadow /= 9;
    return shadow;
}

float CalculateShadow()
{
    float bias = 0.0002;
    vec3 projCoords = lightSpace.xyz / lightSpace.w;
    float currentDepth = projCoords.z - bias;
    if(currentDepth <= 0.0 || currentDepth >= 1.0)
        return 1;

    projCoords = projCoords * 0.5 + 0.5;
    return CalcPCF(projCoords, currentDepth);
}

void main() 
{
    outColor = texture(diffuseSampler, inFragTexCoord) * CalculateShadow();
}