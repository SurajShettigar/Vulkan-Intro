@echo off
 
set buildDir=%~dp0..\build
set objDir=.\obj\
set assetDir=.\assets\
set shaderDir=.\shaders\
 
if exist %buildDir% (  
  pushd %buildDir%
  del /q /s *.exe *.pdb *.ilk *.dll *.spv
  rd /s /q %objDir%
  if exist %assetDir% rd /s /q %assetDir%
  if exist %shaderDir% rd /s /q %shaderDir%
  popd
)