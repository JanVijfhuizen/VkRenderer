#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(set = 0, binding = 1) uniform Light
{
	vec3 pos;
    float range;
} light;

layout (location = 0) in vec4 inFragPos;

void main() 
{
    float lightDistance = length(inFragPos.xyz - light.pos);
    lightDistance = lightDistance / light.range;
    gl_FragDepth = lightDistance;
}