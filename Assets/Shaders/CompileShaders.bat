
set SHADER_COMPILER="%VULKAN_SDK%/Bin/glslangValidator.exe"

%SHADER_COMPILER% -V Shader.vert
%SHADER_COMPILER% -V Shader.frag

pause
