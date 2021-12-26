#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inFragTexCoord;
layout(location = 2) in vec4 lightSpace;

layout (set = 1, binding = 1) uniform sampler2D shadowMap;
layout (set = 2, binding = 0) uniform sampler2D diffuseSampler;

layout(location = 0) out vec4 outColor;

float CalculateShadow()
{
    float bias = 0.0001;
    vec3 projCoords = lightSpace.xyz / lightSpace.w;
    float currentDepth = projCoords.z;
    if(currentDepth <= bias || currentDepth >= 1.0 - bias)
        return 1;

    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float shadow = currentDepth - bias > closestDepth ? 1 : 0;
    return shadow;
}

void main() 
{
    outColor = texture(diffuseSampler, inFragTexCoord) * CalculateShadow();
}