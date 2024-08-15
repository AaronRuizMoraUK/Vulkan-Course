#version 450 // Use GLSL 4.5

// Input interpolated color from vertex
layout(location = 0) in vec4 fragColor;

// Output color for fragment
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = fragColor;
}
