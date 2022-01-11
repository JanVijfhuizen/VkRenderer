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
    float clipFar;
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
    float clipFar = camera.clipFar;

    vec3 pos = pushConstants.position - camera.position;
    vec3 vertPos = inPosition * pushConstants.scale;

    // Scale 2d position based on distance.
    float dis = pos.z + vertPos.z;
    vec2 pos2d = pos.xy + vertPos.xy;
    pos2d /= dis;

    vec3 finalPos = vec3(pos2d, dis / clipFar);
    gl_Position = vec4(finalPos, clipFar);

    outData.normal = inNormal;
    outData.fragTexCoord = inTexCoords;
}