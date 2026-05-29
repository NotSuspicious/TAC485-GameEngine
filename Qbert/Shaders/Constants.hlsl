// We want to use row major matrices
#pragma pack_matrix(row_major)

cbuffer PerObjectConstants : register(b0, space1)
{
    float4x4 c_modelToWorld;
};

cbuffer PerCameraConstants : register(b1, space1)
{
    float4x4 c_viewProj;
};

cbuffer MaterialConstants : register(b0, space3)
{
    float3 c_diffuseColor;
    float padding0;
    float3 c_specularColor;
    float c_specularPower;
};

cbuffer QbertConstants : register(b3, space1)
{
    float3 primaryColor;
    float padding5;
    float3 secondaryColor;
    float padding6;
    float3 activatedColor;
    bool isActivated;
};

#define MAX_POINT_LIGHTS 8
struct PointLightData
{
    float3 lightColor;
    float padding0;
    float3 position;
    float padding1;
    float innerRadius;
    float outerRadius;
    bool isEnabled;
    float padding2;
};  // This struct is now 48 bytes (3 float4s)

cbuffer LightingConstants : register(b1, space3)
{
    float3 c_cameraPosition;
    float padding3;
    float3 c_ambient;
    float padding4;
    PointLightData c_pointLight[MAX_POINT_LIGHTS];
};

#define MAX_SKELETON_BONES 80
cbuffer SkinConstants : register(b2, space1)
{
float4x4 c_skinMatrix[MAX_SKELETON_BONES];
};

SamplerState DiffuseSampler : register(s0, space2);
SamplerState NormalSampler : register(s1, space2);
Texture2D DiffuseTexture : register(t0, space2);
Texture2D NormalTexture : register(t1, space2);
