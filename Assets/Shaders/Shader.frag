#version 450 // Use GLSL 4.5

// Fragment Inputs from interpolating Vertex Outputs
layout(location = 0) in vec2 fragInUV;
layout(location = 1) in vec3 fragInNormal;
layout(location = 2) in vec3 fragInTangent;
layout(location = 3) in vec3 fragInBinormal;
layout(location = 4) in vec3 fragInViewDir;

// Fragment Outputs
layout(location = 0) out vec4 fragOutColor;

layout(push_constant) uniform WorldBuffer
{
    mat4 worldMatrix;
    mat4 inverseTransposeWorldMatrix;
} worldBuffer;

layout(set = 1, binding = 0) uniform sampler imageSampler;
layout(set = 1, binding = 1) uniform texture2D diffuseImage;
layout(set = 1, binding = 2) uniform texture2D emissiveImage;
layout(set = 1, binding = 3) uniform texture2D normalImage;

#define saturate(value)      clamp((value), 0.0, 1.0)

const vec4 LightDir = vec4(1.0, -1.0, 1.0, 0.0);
const vec4 LightColor = vec4(1.0, 1.0, 1.0, 1.0);

const vec3 AmbientColor = vec3(0.0, 0.0, 0.0);
const vec3 BaseDiffuseAmount = vec3(0.05, 0.05, 0.05);
const vec3 SpecularColor = vec3(1.0, 1.0, 1.0);
const float SpecularPower = 70.0;
const float SpecularStrength = 0.8;

const float Gamma = 2.2;
const float InvGamma = 1.0 / Gamma;

void main()
{
    const vec3 lightDir = -normalize(LightDir.xyz);
    
    const vec3 halfDir = normalize(normalize(fragInViewDir) + lightDir.xyz);
    const vec4 diffuleColor = texture(sampler2D(diffuseImage, imageSampler), fragInUV);
    const vec3 emissiveColor = texture(sampler2D(emissiveImage, imageSampler), fragInUV).xyz;
    const vec3 normalColor = texture(sampler2D(normalImage, imageSampler), fragInUV).xyz;
    
    // Normal map
    const mat3 tangentToLocal = mat3(
        normalize(fragInTangent), 
        normalize(fragInBinormal),
        normalize(fragInNormal));
    const vec3 normalTangentSpace = normalize(normalColor * 2.0f - 1.0f);
    vec3 normal = tangentToLocal * normalTangentSpace;
    normal = normalize(mat3(worldBuffer.inverseTransposeWorldMatrix) * normal);
    
    // Diffuse Color
    const vec3 diffuleColorLinear = pow(diffuleColor.rgb, vec3(Gamma));
    const float diffuseAmount = saturate(dot(lightDir.xyz, normal));
    const vec3 diffuse = diffuleColorLinear * max(vec3(diffuseAmount), BaseDiffuseAmount);
    
    // Specular Color
    float specularAmount = SpecularStrength * pow(saturate(dot(halfDir, normal)), SpecularPower);
    specularAmount = saturate(4 * diffuseAmount) * saturate(specularAmount);
    const vec3 specular = SpecularColor * specularAmount;
    
    // Emissive Color
    const vec3 emissiveColorLinear = pow(emissiveColor, vec3(Gamma));
    
    // Final Color and Gamma
    const vec3 colorLinear = LightColor.rgb * (specular + diffuse) + AmbientColor + emissiveColorLinear;
    const vec3 colorGammaCorrected = saturate(pow(colorLinear, vec3(InvGamma)));

    fragOutColor = vec4(colorGammaCorrected, diffuleColor.a);
}
