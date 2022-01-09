#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

layout(location = 0) in Data
{
    vec3 normal;
    vec2 fragTexCoord;
} inData;

layout(location = 0) out vec4 outColor;

void main() 
{
    outColor = vec4(1, 0, 0, 1);
}