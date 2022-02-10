#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

#define LIGHT_COUNT 6

// Light mapping.
layout (set = 0, binding = 0) uniform Light
{
    vec3 pos;
    float range;
} lights[LIGHT_COUNT];
layout (set = 0, binding = 1) uniform samplerCube lightMaps[LIGHT_COUNT];

// Material.
layout (set = 2, binding = 0) uniform sampler2D diffuseSampler;

layout(location = 0) in Data
{
    vec3 normal;
    vec2 fragTexCoord;
    vec3 fragPos;
} inData;

layout(location = 0) out vec4 outColor;

float ShadowCalculation()
{
    vec3 fragToLight = inData.fragPos - lights[0].pos; 
    float closestDepth = texture(lightMaps[0], fragToLight).r;
    closestDepth *= lights[0].range;

    float currentDepth = length(fragToLight);  
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0; 

    return shadow;
}

void main() 
{
    vec4 color = texture(diffuseSampler, inData.fragTexCoord);
    if(color.a < .01f)
        discard;
    outColor = color * (1.0 - ShadowCalculation());
}