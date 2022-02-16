#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

// Light mapping.
layout (set = 0, binding = 0) uniform Lights
{
    Light values[LIGHT_COUNT];
} lights;

layout (set = 0, binding = 1) uniform LightInfo
{
    int count;
} lightInfo;

layout (set = 0, binding = 2) uniform samplerCube lightMaps[LIGHT_COUNT];

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
    float shadowCoverage = 0;

    for(int i = 0; i < lightInfo.count; ++i)
    {
        vec3 fragToLight = inData.fragPos - lights.values[i].pos;
        fragToLight.z *= -1;
        float closestDepth = texture(lightMaps[i], fragToLight).r;
        closestDepth *= lights.values[i].range;

        float currentDepth = length(fragToLight);  
        float bias = 0.05; 
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
        shadowCoverage += shadow;
    }

    shadowCoverage /= lightInfo.count;
    return shadowCoverage;
}

void main() 
{
    vec4 color = texture(diffuseSampler, inData.fragTexCoord);
    if(color.a < .01f)
        discard;
    outColor = color * (1.0 - ShadowCalculation());
}