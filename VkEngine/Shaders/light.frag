#version 450
#extension GL_KHR_vulkan_glsl : enable
#include "shader.glsl"

layout (set = 0, binding = 1) uniform Lights
{
    Light values[LIGHT_COUNT];
} lights;

layout(location = 0) in flat int inIndex;
layout (location = 1) in vec4 inFragPos;

void main() 
{
    float lightDistance = length(inFragPos.xyz - lights.values[inIndex].pos);
    lightDistance = lightDistance / lights.values[inIndex].range;
    gl_FragDepth = lightDistance;
}