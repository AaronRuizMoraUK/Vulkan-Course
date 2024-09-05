#version 450 // Use GLSL 4.5

// Vertex Inputs
layout(location = 0) in vec3 vertexInPosition;
layout(location = 1) in vec3 vertexInNormal;
layout(location = 2) in vec3 vertexInTangent;
layout(location = 3) in vec3 vertexInBinormal;
layout(location = 4) in vec2 vertexInUV;

// Vertex Outputs
layout(location = 0) out vec2 vertexOutUV;
layout(location = 1) out vec3 vertexOutNormal;
layout(location = 2) out vec3 vertexOutTangent;
layout(location = 3) out vec3 vertexOutBinormal;
layout(location = 4) out vec3 vertexOutViewDir;

layout(set = 0, binding = 0) uniform ViewProjBuffer
{
    mat4 viewMatrix;
    mat4 projMatrix;
    vec4 camPos;
} viewProjBuffer;

layout(push_constant) uniform WorldBuffer
{
    mat4 worldMatrix;
    mat4 inverseTransposeWorldMatrix;
} worldBuffer;

void main()
{
    gl_Position = worldBuffer.worldMatrix * vec4(vertexInPosition, 1.0);
    vertexOutViewDir = viewProjBuffer.camPos.xyz - gl_Position.xyz;
    gl_Position = viewProjBuffer.projMatrix * viewProjBuffer.viewMatrix * gl_Position;
    vertexOutNormal = vertexInNormal;
    vertexOutTangent = vertexInTangent;
    vertexOutBinormal = vertexInBinormal;
    vertexOutUV = vertexInUV;
}
