#include "Constants.hlsl"
#include "Lighting.hlsl"

struct VIn
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float4 worldPosition : TEXCOORD1;
};

VOut VS(VIn vIn)
{
    VOut output;
    output.position = mul(float4(vIn.position, 1.0), c_modelToWorld);
    output.worldPosition = output.position;
    output.normal = mul(float4(vIn.normal, 0.0), c_modelToWorld);
    output.position = mul(output.position, c_viewProj);
    output.uv = vIn.uv;
    return output;
}

float4 PS(VOut pIn) : SV_TARGET
{
    float3 normal = normalize(pIn.normal.xyz);
    float3 toViewer = normalize(c_cameraPosition - pIn.worldPosition.xyz);

    float4 finalColor = CalculateLighting(normal, pIn.worldPosition.xyz, toViewer);

    float4 texColor = DiffuseTexture.Sample(DiffuseSampler, pIn.uv);
    return finalColor * texColor * float4(c_diffuseColor, 1.0);
}
