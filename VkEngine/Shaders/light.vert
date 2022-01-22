#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

layout(location = 0) in int inPositionIndex;

layout (set = 0, binding = 0) uniform Camera
{
    mat4 view;
    mat4 projection;
} camera;

layout (push_constant) uniform PushConstants
{
    vec2 vertices[3];
    vec2 textureCoordinates[3];
    float height;
} pushConstants;

layout(location = 0) out Data
{
    vec2 fragTexCoord;
} outData;

void main() 
{
    outData.fragTexCoord = pushConstants.textureCoordinates[inPositionIndex];

    gl_Position = camera.projection * camera.view * mat4(1) * vec4(pushConstants.vertices[inPositionIndex], pushConstants.height, 1);
}