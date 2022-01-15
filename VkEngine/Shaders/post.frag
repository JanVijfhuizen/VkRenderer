#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform sampler2D diffuseSampler;

layout(location = 0) in Data
{
    vec2 fragTexCoord;
} inData;

void main() 
{
    outColor = texture(diffuseSampler, inData.fragTexCoord);
}