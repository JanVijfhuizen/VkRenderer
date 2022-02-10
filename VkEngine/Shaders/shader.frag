#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

#define LIGHT_COUNT 6

// Light mapping.
layout (set = 0, binding = 0) uniform LightMapInfo
{
    float f;
} lightMapInfos[LIGHT_COUNT];
layout (set = 0, binding = 1) uniform samplerCube lightMaps[LIGHT_COUNT];

// Material.
layout (set = 2, binding = 0) uniform sampler2D diffuseSampler;

layout(location = 0) in Data
{
    vec3 normal;
    vec2 fragTexCoord;
} inData;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec4 color = texture(diffuseSampler, inData.fragTexCoord);
    if(color.a < .01f)
        discard;
    outColor = color;
}