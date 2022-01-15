#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D diffuseSampler;
layout (set = 0, binding = 1) uniform sampler2D depthSampler;

layout(location = 0) in Data
{
    vec2 fragTexCoord;
} inData;

void main() 
{
    gl_FragDepth = texture(depthSampler, inData.fragTexCoord).x;
    outColor = texture(diffuseSampler, inData.fragTexCoord);
}