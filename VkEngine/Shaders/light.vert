#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

layout(location = 0) in int inPositionIndex;
layout(location = 1) in vec2 inTexCoords;

layout (set = 0, binding = 0) uniform Camera
{
    mat4 view;
    mat4 projection;
    float clipFar;
    float aspectRatio;
} camera;

layout (push_constant) uniform PushConstants
{
    vec3 vertices[4];
} pushConstants;

layout(location = 0) out Data
{
    vec2 fragTexCoord;
} outData;

void main() 
{
    outData.fragTexCoord = inTexCoords;

    gl_Position = camera.projection * camera.view * mat4(1) * vec4(pushConstants.vertices[inPositionIndex], 1);
}