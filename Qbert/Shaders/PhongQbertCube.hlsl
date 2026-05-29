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
    float3 qbertColor : COLOR0;
};

VOut VS(VIn vIn)
{
    VOut output;
    output.position = mul(float4(vIn.position, 1.0), c_modelToWorld);
    output.worldPosition = output.position;
    output.normal = mul(float4(vIn.normal, 0.0), c_modelToWorld);
    output.position = mul(output.position, c_viewProj);
    output.uv = vIn.uv;

    // Determine face color in vertex shader (all vertices of a face have same normal)
    float3 normal = normalize(output.normal.xyz);
    float3 up = float3(0.0, 0.0, 1.0);
    if (dot(normal, up) >= 1.0)
    {
        output.qbertColor = isActivated ? activatedColor : primaryColor;
    }
    else
    {
        output.qbertColor = secondaryColor;
    }

    return output;
}

float4 PS(VOut pIn) : SV_TARGET
{
    // Use world-space normal from vertex shader for lighting
    float3 normal = normalize(pIn.normal.xyz);
    float3 toViewer = normalize(c_cameraPosition - pIn.worldPosition.xyz);

    float4 lighting = CalculateLighting(normal, pIn.worldPosition.xyz, toViewer);

    float4 texColor = DiffuseTexture.Sample(DiffuseSampler, pIn.uv);

    // Use the color determined in vertex shader (constant across the triangle)
    return lighting * texColor * float4(pIn.qbertColor, 1.0);
}
