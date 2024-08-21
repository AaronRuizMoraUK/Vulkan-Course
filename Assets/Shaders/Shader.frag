#version 450 // Use GLSL 4.5

// Fragment Inputs from interpolating Vertex Outputs
layout(location = 0) in vec4 fragInColor;

// Fragment Outputs
layout(location = 0) out vec4 fragOutColor;

void main()
{
    fragOutColor = fragInColor;
}
