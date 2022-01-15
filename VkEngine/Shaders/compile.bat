%~dp0/glslc.exe shader.vert -o vert.spv
%~dp0/glslc.exe shader.frag -o frag.spv

%~dp0/glslc.exe post.vert -o post-vert.spv
%~dp0/glslc.exe post.frag -o post-frag.spv

pause