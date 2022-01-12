#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

layout(location = 0) in Data
{
    vec3 normal;
    vec2 fragTexCoord;
} inData;

layout(location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;

void main() 
{
    outColor = texture(diffuseSampler, inData.fragTexCoord);
}