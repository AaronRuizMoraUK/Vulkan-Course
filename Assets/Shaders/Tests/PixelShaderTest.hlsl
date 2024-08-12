struct PixelIn
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float2 uv : TEXCOORD0;
};

struct PixelOut
{
    float4 color : SV_Target;
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

SamplerState samplerState : register(s0);

Texture1D tex1D : register(t1);
Texture2D tex2D : register(t2);
Texture3D tex3D : register(t3);
TextureCube texCube : register(t4);
Texture1DArray tex1DArray : register(t5);
Texture2DArray tex2DArray : register(t6);
TextureCubeArray texCubeArray : register(t7);

// UAV registers live in the same name space as pixel shader outputs,
// so in pixel shader needs to start from register (0 + output render targets).
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

RasterizerOrderedBuffer<float4> bufferROTyped : register(u9);
RasterizerOrderedStructuredBuffer<MySB> structuredROBuffer : register(u10);
RasterizerOrderedByteAddressBuffer rawROBuffer : register(u11);

// ---------------------------------------------------------------------
// Samplers, Textures and Buffers can also be declared as arrays,
// the register indicated would be the start point.

static const int ArrayCount = 3;

SamplerState samplerStates[ArrayCount] : register(s1);

Texture2D tex2Ds[ArrayCount] : register(t11);
Texture2DArray tex2DArrays[ArrayCount] : register(t14);

RWTexture2D<float4> texRW2Ds[ArrayCount] : register(u12);
RWTexture2DArray<float4> texRW2DArrays[ArrayCount] : register(u15);

Buffer<float4> buffersTyped[ArrayCount] : register(t17);
StructuredBuffer<MySB> structuredBuffers[ArrayCount] : register(t20);
ByteAddressBuffer rawBuffers[ArrayCount] : register(t23);

// ---------------------------------------------------------------------
PixelOut main(PixelIn pixelIn)
{
    PixelOut pixelOut;
    pixelOut.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    pixelOut.color = mul(worldMatrixCBuffer, pixelOut.color);
    pixelOut.color = mul(worldMatrixTBuffer, pixelOut.color);
    
    pixelOut.color *= tex1D.Sample(samplerState, 0.0);
    pixelOut.color *= tex2D.Sample(samplerState, float2(0.0, 0.0));
    pixelOut.color *= tex3D.Sample(samplerState, float3(0.0, 0.0, 0.0));
    pixelOut.color *= texCube.Sample(samplerState, float3(0.0, 0.0, 1.0));
    pixelOut.color *= tex1DArray.Sample(samplerState, float2(0.0, 0.0)); // y is array index
    pixelOut.color *= tex2DArray.Sample(samplerState, float3(0.0, 0.0, 0.0)); // z is array index
    pixelOut.color *= texCubeArray.Sample(samplerState, float4(0.0, 0.0, 1.0, 0.0f)); // w is array index
    
    pixelOut.color *= texRW1D[0];
    pixelOut.color *= texRW2D[uint2(0, 0)];
    pixelOut.color *= texRW3D[uint3(0, 0, 0)];
    pixelOut.color *= texRW1DArray[uint2(0, 0)]; // y is array index
    pixelOut.color *= texRW2DArray[uint3(0, 0, 0)]; // z is array index
    
    texRW1D[0] = pixelOut.color;
    texRW2D[uint2(0, 0)] = pixelOut.color;
    texRW3D[uint3(0, 0, 0)] = pixelOut.color;
    texRW1DArray[uint2(0, 0)] = pixelOut.color; // y is array index
    texRW2DArray[uint3(0, 0, 0)] = pixelOut.color; // z is array index
    
    pixelOut.color *= bufferTyped[0];
    pixelOut.color *= bufferRWTyped[0];
    pixelOut.color *= bufferROTyped[0];
    
    pixelOut.color = mul(structuredBuffer[0].worldMatrix, pixelOut.color);
    pixelOut.color = mul(structuredRWBuffer[0].worldMatrix, pixelOut.color);
    pixelOut.color = mul(structuredROBuffer[0].worldMatrix, pixelOut.color);
    
    pixelOut.color.a *= (float) rawBuffer.Load(0); // Load uint from byte 0
    pixelOut.color.a *= (float) rawRWBuffer.Load(0);
    pixelOut.color.a *= (float) rawROBuffer.Load(0);

    // ---------------------------------------------------------------------
    
    for (int i = 0; i < ArrayCount; ++i)
    {
        pixelOut.color *= tex2Ds[i].Sample(samplerStates[0], float2(0.0, 0.0));
        pixelOut.color *= tex2DArrays[i].Sample(samplerStates[1], float3(0.0, 0.0, 0.0)); // z is array index
    
        pixelOut.color *= texRW2Ds[i][uint2(0, 0)];
        pixelOut.color *= texRW2DArrays[i][uint3(0, 0, 0)]; // z is array index
    
        pixelOut.color *= buffersTyped[i][0];
        pixelOut.color = mul(structuredBuffers[i][0].worldMatrix, pixelOut.color);
        pixelOut.color.a *= (float) rawBuffers[i].Load(0); // Load uint from byte 0
    }
    
    return pixelOut;
}
