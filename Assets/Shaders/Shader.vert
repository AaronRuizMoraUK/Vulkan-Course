#version 450 // Use GLSL 4.5

// Triangle vertex positions (will put in to vertex buffer later!)
vec3 VertexPositions[3] = vec3[](
    vec3(0.0, -0.4, 0.0),
    vec3(0.4, 0.4, 0.0),
    vec3(-0.4, 0.4, 0.0)
);

// Triangle vertex colors
vec4 VertexColors[3] = vec4[](
    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0)
);

// Output color for vertex
layout(location = 0) out vec4 fragColor;

void main()
{
    gl_Position = vec4(VertexPositions[gl_VertexIndex], 1.0);
    fragColor = VertexColors[gl_VertexIndex];
}
