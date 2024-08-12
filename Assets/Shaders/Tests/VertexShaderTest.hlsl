struct VertexIn
{
    float3 position : SV_Position;
    float2 uv : TEXCOORD0;
};

struct VertexOut
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

// ---------------------------------------------------------------------
// Test all resources types
// ---------------------------------------------------------------------
cbuffer CBuffer : register(b0)
{
    float4x4 worldMatrixCBuffer;
};

tbuffer TBuffer : register(t0)
{
    float4x4 worldMatrixTBuffer;
};

// SamplerState are not supported in Vertex Shader
// TextureCube and TextureCubeArray are not supported in Vertex Shader
// RasterizerOrdered buffers are not supported in Vertex Shader

Texture1D tex1D : register(t1);
Texture2D tex2D : register(t2);
Texture3D tex3D : register(t3);
Texture1DArray tex1DArray : register(t5);
Texture2DArray tex2DArray : register(t6);

RWTexture1D<float4> texRW1D : register(u1);
RWTexture2D<float4> texRW2D : register(u2);
RWTexture3D<float4> texRW3D : register(u3);
RWTexture1DArray<float4> texRW1DArray : register(u4);
RWTexture2DArray<float4> texRW2DArray : register(u5);

struct MySB
{
    float4x4 viewMatrix;
    float4x4 projMatrix;
    float4x4 worldMatrix;
};

Buffer<float4> bufferTyped : register(t8);
StructuredBuffer<MySB> structuredBuffer : register(t9);
ByteAddressBuffer rawBuffer : register(t10);

RWBuffer<float4> bufferRWTyped : register(u6);
RWStructuredBuffer<MySB> structuredRWBuffer : register(u7);
RWByteAddressBuffer rawRWBuffer : register(u8);

// ---------------------------------------------------------------------
// Samplers, Textures and Buffers can also be declared as arrays,
// the register indicated would be the start point.

static const int ArrayCount = 3;

Texture2D tex2Ds[ArrayCount] : register(t11);
Texture2DArray tex2DArrays[ArrayCount] : register(t14);

RWTexture2D<float4> texRW2Ds[ArrayCount] : register(u12);
RWTexture2DArray<float4> texRW2DArrays[ArrayCount] : register(u15);

Buffer<float4> buffersTyped[ArrayCount] : register(t17);
StructuredBuffer<MySB> structuredBuffers[ArrayCount] : register(t20);
ByteAddressBuffer rawBuffers[ArrayCount] : register(t23);

VertexOut main(VertexIn vertexIn)
{
    VertexOut vertexOut;
    vertexOut.uv = vertexIn.uv;
    vertexOut.position = float4(vertexIn.position, 1.0);
    vertexOut.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    vertexOut.color = mul(worldMatrixCBuffer, vertexOut.color);
    vertexOut.color = mul(worldMatrixTBuffer, vertexOut.color);
    
    vertexOut.color *= tex1D.Load(int2(0, 0)); // y is mip level
    vertexOut.color *= tex2D.Load(int3(0, 0, 0)); // z is mip level
    vertexOut.color *= tex3D.Load(int4(0, 0, 0, 0)); // w is mip level
    vertexOut.color *= tex1DArray.Load(int3(0, 0, 0)); // y is mip level, z is array index
    vertexOut.color *= tex2DArray.Load(int4(0, 0, 0, 0)); // z is mip level, w is array index
    
    vertexOut.color *= texRW1D[0];
    vertexOut.color *= texRW2D[uint2(0, 0)];
    vertexOut.color *= texRW3D[uint3(0, 0, 0)];
    vertexOut.color *= texRW1DArray[uint2(0, 0)]; // y is array index
    vertexOut.color *= texRW2DArray[uint3(0, 0, 0)]; // z is array index
    
    texRW1D[0] = vertexOut.color;
    texRW2D[uint2(0, 0)] = vertexOut.color;
    texRW3D[uint3(0, 0, 0)] = vertexOut.color;
    texRW1DArray[uint2(0, 0)] = vertexOut.color; // y is array index
    texRW2DArray[uint3(0, 0, 0)] = vertexOut.color; // z is array index
    
    vertexOut.color *= bufferTyped[0];
    vertexOut.color *= bufferRWTyped[0];
    
    vertexOut.position = mul(structuredBuffer[0].worldMatrix, vertexOut.position);
    vertexOut.position = mul(structuredRWBuffer[0].worldMatrix, vertexOut.position);
    
    vertexOut.color.a *= (float) rawBuffer.Load(0); // Load uint from byte 0
    vertexOut.color.a *= (float) rawRWBuffer.Load(0);

    // ---------------------------------------------------------------------
    
    for (int i = 0; i < ArrayCount; ++i)
    {
        vertexOut.color *= tex2Ds[i].Load(int3(0, 0, 0)); // z is mip level
        vertexOut.color *= tex2DArrays[i].Load(int4(0, 0, 0, 0)); // z is mip level, w is array index
        
        vertexOut.color *= texRW2Ds[i][uint2(0, 0)];
        vertexOut.color *= texRW2DArrays[i][uint3(0, 0, 0)]; // z is array index
        
        vertexOut.color *= buffersTyped[i][0];
        vertexOut.color = mul(structuredBuffers[i][0].worldMatrix, vertexOut.color);
        vertexOut.color.a *= (float) rawBuffers[i].Load(0); // Load uint from byte 0
    }

    return vertexOut;
}
