#include "Constants.hlsl"
#include "Lighting.hlsl"

struct VIn
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    uint4 boneIndices : BLENDINDICES0;
    float4 boneWeights : BLENDWEIGHT0;
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

    for (int i = 0 ; i < 4; i++){
        uint boneIndex = vIn.boneIndices[i];
        float weight = vIn.boneWeights[i];
        if (weight > 0){
            float4x4 skinMatrix = c_skinMatrix[boneIndex];
            output.position += weight * mul(float4(vIn.position, 1.0), skinMatrix);
            output.normal += weight * mul(float4(vIn.normal, 0.0), skinMatrix);
        }
    }

    output.position = mul(output.position, c_modelToWorld);
    output.worldPosition = output.position;

    output.normal = mul(output.normal, c_modelToWorld);
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
    return finalColor * texColor;
}
