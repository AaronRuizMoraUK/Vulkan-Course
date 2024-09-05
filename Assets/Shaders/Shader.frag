#version 450 // Use GLSL 4.5

// Fragment Inputs from interpolating Vertex Outputs
layout(location = 0) in vec2 fragInUV;

// Fragment Outputs
layout(location = 0) out vec4 fragOutColor;

layout(set = 1, binding = 0) uniform sampler imageSampler;
layout(set = 1, binding = 1) uniform texture2D diffuseImage;
layout(set = 1, binding = 2) uniform texture2D emissiveImage;
layout(set = 1, binding = 3) uniform texture2D normalImage;

void main()
{
    fragOutColor = texture(sampler2D(diffuseImage, imageSampler), fragInUV) +
                   texture(sampler2D(emissiveImage, imageSampler), fragInUV);
}
