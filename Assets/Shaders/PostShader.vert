#version 450 // Use GLSL 4.5

// Triangle vertex positions covering the entire screen
vec2 VertexPositions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2(3.0, -1.0),
    vec2(-1.0, 3.0)
);

void main()
{
    gl_Position = vec4(VertexPositions[gl_VertexIndex], 0.0, 1.0);
}
