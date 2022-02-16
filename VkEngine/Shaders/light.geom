#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

layout(set = 0, binding = 0) uniform Matrices
{
	mat4 values[6];
} matrices;

layout (location = 0) in flat int[] inIndex;

layout (location = 0) out int outIndex;
layout (location = 1) out vec4 outFragPos;

void main(void)
{
	for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            outFragPos = gl_in[i].gl_Position;
            gl_Position = matrices.values[face] * outFragPos;
            outIndex = inIndex[i];
            EmitVertex();
        }    
        EndPrimitive();
    }
}