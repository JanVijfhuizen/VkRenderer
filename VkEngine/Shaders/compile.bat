%~dp0/glslc.exe shader.vert -o vert.spv
%~dp0/glslc.exe shader.frag -o frag.spv

%~dp0/glslc.exe post.vert -o post-vert.spv
%~dp0/glslc.exe post.frag -o post-frag.spv

%~dp0/glslc.exe light.vert -o light-vert.spv
%~dp0/glslc.exe light.geom -o light-geom.spv
%~dp0/glslc.exe light.frag -o light-frag.spv

pause