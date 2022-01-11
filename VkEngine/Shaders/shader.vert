#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords;

layout (set = 0, binding = 0) uniform Camera
{
    vec3 position;
    float rotation;
    float depth;
} camera;

layout (push_constant) uniform PushConstants
{
    vec3 position;
    float rotation;
    float scale;
} pushConstants;

layout(location = 0) out Data
{
    vec3 normal;
    vec2 fragTexCoord;
} outData;

void main() 
{
    gl_Position = vec4(pushConstants.position + inPosition - camera.position, camera.depth);

    outData.normal = inNormal;
    outData.fragTexCoord = inTexCoords;
}