#include "Constants.hlsl"
#include "Lighting.hlsl"

struct VIn
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float2 uv : TEXCOORD0;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 normal : NORMAL0;
    float4 tangent : TANGENT0;
    float2 uv : TEXCOORD0;
    float4 worldPosition : TEXCOORD1;
};

VOut VS(VIn vIn)
{
    VOut output;
    output.position = mul(float4(vIn.position, 1.0), c_modelToWorld);
    output.worldPosition = output.position;

    float3x3 normalMatrix = (float3x3)c_modelToWorld;
    output.normal = float4(mul(vIn.normal, normalMatrix), 0.0);
    output.tangent = float4(mul(vIn.tangent, normalMatrix), 0.0);

    output.position = mul(output.position, c_viewProj);
    output.uv = vIn.uv;
    return output;
}

float4 PS(VOut pIn) : SV_TARGET
{
    float3 normal = normalize(pIn.normal.xyz);
    float3 tangent = normalize(pIn.tangent.xyz);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    float3 biNormal = normalize(cross(normal,tangent));
    float3x3 TBN = { tangent, biNormal, normal};

    // Sample normal map and convert from [0,1] to [-1,1] range
    float3 sampledNormal = NormalTexture.Sample(NormalSampler, pIn.uv).xyz;
    sampledNormal = normalize(sampledNormal * 2.0 - 1.0);

    normal = normalize(mul(sampledNormal, TBN));

    float3 toViewer = normalize(c_cameraPosition - pIn.worldPosition.xyz);

    float4 finalColor = CalculateLighting(normal, pIn.worldPosition.xyz, toViewer);

    float4 texColor = DiffuseTexture.Sample(DiffuseSampler, pIn.uv);
    return finalColor * texColor;
}
