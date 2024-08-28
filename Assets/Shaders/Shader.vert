#version 450 // Use GLSL 4.5

// Vertex Inputs
layout(location = 0) in vec3 vertexInPosition;
layout(location = 1) in vec4 vertexInColor;

// Vertex Outputs
layout(location = 0) out vec4 vertexOutColor;

layout(set = 0, binding = 0) uniform ViewProjUniformBuffer
{
    mat4 viewMatrix;
    mat4 projMatrix;
    vec4 camPos;
} viewProjBuffer;

layout(set = 1, binding = 0) uniform WorldUniformBuffer
{
    mat4 worldMatrix;
    mat4 inverseTransposeWorldMatrix;
} worldBuffer;

void main()
{
    gl_Position = viewProjBuffer.projMatrix * viewProjBuffer.viewMatrix * worldBuffer.worldMatrix * vec4(vertexInPosition, 1.0);
    vertexOutColor = vertexInColor;
}
