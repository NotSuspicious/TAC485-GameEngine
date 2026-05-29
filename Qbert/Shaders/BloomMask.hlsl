#include "Constants.hlsl"
#include "Lighting.hlsl"

struct VIn
{
    float3 position : POSITION0;
    float2 uv : TEXCOORD0;
};

struct VOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VOut VS(VIn vIn)
{
    VOut output;

    output.position = float4(vIn.position, 1);
    output.uv = vIn.uv;

    return output;
}

float4 PS(VOut pIn) : SV_TARGET
{
    float3 color = DiffuseTexture.Sample(DiffuseSampler, pIn.uv).xyz;
    if (length(color) < 0.99){
        return float4(0,0,0,1);
    }
    return float4(color,0.0);
}
