#version 450 // Use GLSL 4.5

// Vertex Inputs
layout(location = 0) in vec3 vertexInPosition;
layout(location = 1) in vec4 vertexInColor;

// Vertex Outputs
layout(location = 0) out vec4 vertexOutColor;

// Uniform Buffer Objects
layout(set = 0, binding = 0) uniform WorldViewProjMatrixUniformBuffer
{
    mat4 worldMatrix;
    mat4 viewMatrix;
    mat4 projMatrix;
} wvp;

void main()
{
    gl_Position = wvp.projMatrix * wvp.viewMatrix * wvp.worldMatrix * vec4(vertexInPosition, 1.0);
    vertexOutColor = vertexInColor;
}
