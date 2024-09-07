
set SHADER_COMPILER="%VULKAN_SDK%/Bin/glslangValidator.exe"

%SHADER_COMPILER% -V Shader.vert -o Shader.vert.spv
%SHADER_COMPILER% -V Shader.frag -o Shader.frag.spv

%SHADER_COMPILER% -V PostShader.vert -o PostShader.vert.spv
%SHADER_COMPILER% -V PostShader.frag -o PostShader.frag.spv

pause
