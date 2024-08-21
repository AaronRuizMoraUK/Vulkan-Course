#version 450 // Use GLSL 4.5

// Vertex Inputs
layout(location = 0) in vec3 vertexInPosition;
layout(location = 1) in vec4 vertexInColor;

// Vertex Outputs
layout(location = 0) out vec4 vertexOutColor;

void main()
{
    gl_Position = vec4(vertexInPosition, 1.0);
    vertexOutColor = vertexInColor;
}
