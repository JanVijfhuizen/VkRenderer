#version 450
#extension GL_KHR_vulkan_glsl : enable

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
} light;

layout (push_constant) uniform PushConstants
{
    mat4 model;
} pushConstants;

layout(location = 0) out OutData
{
    vec3 normal;
    vec2 fragTexCoord;
    vec4 lightSpace;
    vec3 lightDir;
} outData;

void main() 
{
    gl_Position = camera.projection * camera.view * pushConstants.model * vec4(inPosition, 1);

    outData. normal = inNormal;
    outData.fragTexCoord = inTexCoords;
    outData.lightSpace = light.spaceMatrix * pushConstants.model * vec4(inPosition, 1);
    outData.lightDir = light.dir;
}