struct VertexIn
{
    float3 position : SV_Position;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float2 uv : TEXCOORD0;
};

struct VertexOut
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
    float3 viewDir : TEXCOORD4;
};

cbuffer ViewProjMatrixConstantBuffer : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4 camPos;
};

cbuffer WorldMatrixConstantBuffer : register(b1)
{
    float4x4 worldMatrix;
    float4x4 inverseTransposeWorldMatrix;
};

VertexOut main(VertexIn vertexIn)
{
    VertexOut vertexOut;
    vertexOut.position = mul(worldMatrix, float4(vertexIn.position, 1.0));
    vertexOut.viewDir = camPos.xyz - vertexOut.position.xyz;
    vertexOut.position = mul(viewMatrix, vertexOut.position);
    vertexOut.position = mul(projMatrix, vertexOut.position);
    vertexOut.normal = vertexIn.normal;
    vertexOut.tangent = vertexIn.tangent;
    vertexOut.binormal = vertexIn.binormal;
    vertexOut.uv = vertexIn.uv;

    return vertexOut;
}