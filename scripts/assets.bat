set buildDir=%~dp0..\build
set assetsDir=%~dp0..\assets
set shaderDir=%~dp0..\src\engine\shaders
set extDir=%~dp0..\external
 
:: Copy dependencies
if not exist %buildDir%\SDL2.dll xcopy /y %VULKAN_SDK%\Third-Party\Bin\SDL2.dll %buildDir%
if not exist %buildDir%\glfw3.dll xcopy /y %extDir%\glfw\lib-vc2019\glfw3.dll %buildDir%
 
:: Copy assets
if not exist %buildDir%\assets mkdir %buildDir%\assets
xcopy /y /s %assetsDir% %buildDir%\assets

:: Copy shaders
if not exist %buildDir%\shaders mkdir %buildDir%\shaders
xcopy /y /s %shaderDir%\*.spv %buildDir%\shaders
