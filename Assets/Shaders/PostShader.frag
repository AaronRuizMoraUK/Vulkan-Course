#version 450 // Use GLSL 4.5

// Fragment Inputs from interpolating Vertex Outputs (none)

// Fragment Inputs Attachments
layout(set = 0, binding = 0, input_attachment_index = 0) uniform subpassInput inputColorImage; // Color output from subpass 0
layout(set = 0, binding = 1, input_attachment_index = 1) uniform subpassInput inputDepthImage; // Depth output from subpass 0

// Fragment Outputs
layout(location = 0) out vec4 fragOutColor;

void main()
{
    const int xHalf = 1280 / 2;

    if (gl_FragCoord.x < xHalf)
    {
        fragOutColor = subpassLoad(inputColorImage).rgba;
    }
    else
    {
        const float lowerBound = 0.98f;
        const float upperBound = 1.0f;

        float depth = subpassLoad(inputDepthImage).r;
        float depthColorScaled = 1.0f - max(depth - lowerBound, 0.0f) / (upperBound - lowerBound);

        fragOutColor = vec4(depthColorScaled, depthColorScaled, depthColorScaled, 1.0f);
    }
}
