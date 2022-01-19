#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

layout(location = 0) in Data
{
    vec2 fragTexCoord;
} inData;

layout(location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform sampler2D diffuseSampler;

void main() 
{
    vec4 color = texture(diffuseSampler, inData.fragTexCoord);
    if(color.a < .01f)
        discard;
    outColor = vec4(vec3(0), 1);
}