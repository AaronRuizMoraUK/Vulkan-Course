#version 450 // Use GLSL 4.5

// Fragment Inputs from interpolating Vertex Outputs
layout(location = 0) in vec2 fragInUV;

// Fragment Outputs
layout(location = 0) out vec4 fragOutColor;

void main()
{
    fragOutColor = vec4(fragInUV.x, fragInUV.y, 0.0f, 1.0f);
}
