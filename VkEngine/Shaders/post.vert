#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec2 inPosition;
layout(location = 2) in vec2 inTexCoords;

layout(location = 0) out Data
{
    vec2 fragTexCoord;
} outData;

void main() 
{
    outData.fragTexCoord = inTexCoords;
    gl_Position = vec4(inPosition, 0, 0);
}