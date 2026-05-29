#include "Constants.hlsl"
#include "Lighting.hlsl"

static const float offset[3] = {0.0, 1.3846153846, 3.2307692308};
static const float weight[3] = {0.2270270270, 0.3162162162, 0.0702702703};

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
    float3 color = DiffuseTexture.Sample(DiffuseSampler, pIn.uv).xyz * weight[0];

    for (int i = 1; i < 3; i++)
    {
        color += DiffuseTexture.Sample(DiffuseSampler, pIn.uv + float2(offset[i] / 200.0, 0.0)).xyz * weight[i];
        color += DiffuseTexture.Sample(DiffuseSampler, pIn.uv - float2(offset[i] / 200.0, 0.0)).xyz * weight[i];
    }

    return float4(color, 1.0);
}
