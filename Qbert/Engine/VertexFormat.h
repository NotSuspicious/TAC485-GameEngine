#pragma once

#include "EngineMath.h"
#include <SDL3/SDL_gpu.h>

// This file contains C++ structures describing the various vertex formats

// For now, I have provided the first one
// This matches the HLSL structure from Simple.hlsl:
//struct VIn
//{
//    float3 position : POSITION0;
//    float4 color : COLOR0;
//};
struct VertexPosColor
{
    Vector3 pos;
    Color4 color;
};

struct VertexPosUV
{
    Vector3 pos;
    Color4 color;
    Vector2 uv;
};

struct VertexData
{
    Vector3 pos;
    Vector3 normal;
    Vector2 uv;
};

struct VertexNormalData
{
    Vector3 pos;
    Vector3 normal;
    Vector3 tangent;
    Vector2 uv;
};

struct VertexDataSkinned
{
    Vector3 pos;
    Vector3 normal;
    uint8_t bones[4];
    uint8_t weights[4];
    Vector2 uv;
};

struct VertexUV
{
    Vector3 pos;
    Vector2 uv;
};
