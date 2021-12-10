@echo off
set shader_dir=%~dp0..\src\engine\shaders
pushd  %shader_dir%

%VULKAN_SDK%\Bin\glslc.exe shader.vert -o shader.vert.spv
%VULKAN_SDK%\Bin\glslc.exe shader.frag -o shader.frag.spv

popd 