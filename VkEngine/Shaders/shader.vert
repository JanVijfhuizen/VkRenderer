#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

layout (set = 0, binding = 0) uniform Camera
{
    mat4 view;
    mat4 projection;
} camera;

layout (set = 1, binding = 0) uniform Light
{
    mat4 spaceMatrix;
    vec3 dir;
} lights[LIGHT_MAX_COUNT];

layout (set = 1, binding = 2) uniform LightInfo
{
    int count;
} lightInfo;

layout (push_constant) uniform PushConstants
{
    mat4 model;
} pushConstants;

layout(location = 0) out Data
{
    vec3 normal;
    vec2 fragTexCoord;
    int count;
    vec4 spaces[LIGHT_MAX_COUNT];
    vec3 dirs[LIGHT_MAX_COUNT];
} outData;

void main() 
{
    gl_Position = camera.projection * camera.view * pushConstants.model * vec4(inPosition, 1);

    outData.normal = inNormal;
    outData.fragTexCoord = inTexCoords;
   
    outData.count = lightInfo.count;
    for(int i = 0; i < lightInfo.count; i++)
    {
        outData.spaces[i] = lights[i].spaceMatrix * pushConstants.model * vec4(inPosition, 1);
        outData.dirs[i] = lights[i].dir;
    }
}