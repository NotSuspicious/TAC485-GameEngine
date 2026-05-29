//
// Created by William Zhao on 2/4/26.
//

#include "TestCube.h"

#include "EngineMath.h"
#include "VertexBuffer.h"
#include "VertexFormat.h"
#include "Renderer.h"
#include "Mesh.h"

TestCube::TestCube(Renderer *renderer, Material *material)
    : Actor(renderer, nullptr), m_renderer(renderer)
{
    //    static VertexData cubeVertex[] =
    //            {
    //                    // Bottom Face (Z = -0.5, normal pointing backward)
    //                    {Vector3(-0.5f, 0.5f, -0.5f),  Vector3(0.0f, 0.0f, -1.0f), Color4(1.0f, 0.0f, 0.0f, 1.0f), Vector2(
    //                            0.0f, 0.0f)},
    //                    {Vector3(0.5f, 0.5f, -0.5f),   Vector3(0.0f, 0.0f, -1.0f), Color4(1.0f, 0.0f, 0.0f, 1.0f), Vector2(
    //                            1.0f, 0.0f)},
    //                    {Vector3(0.5f, -0.5f, -0.5f),  Vector3(0.0f, 0.0f, -1.0f), Color4(1.0f, 0.0f, 0.0f, 1.0f), Vector2(
    //                            1.0f, 1.0f)},
    //                    {Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f), Color4(1.0f, 0.0f, 0.0f, 1.0f), Vector2(
    //                            0.0f, 1.0f)},
    //
    //                    // Top Face (Z = 0.5, normal pointing forward)
    //                    {Vector3(-0.5f, 0.5f, 0.5f),   Vector3(0.0f, 0.0f, 1.0f),  Color4(0.0f, 1.0f, 0.0f, 1.0f), Vector2(
    //                            0.0f, 0.0f)},
    //                    {Vector3(0.5f, 0.5f, 0.5f),    Vector3(0.0f, 0.0f, 1.0f),  Color4(0.0f, 1.0f, 0.0f, 1.0f), Vector2(
    //                            1.0f, 0.0f)},
    //                    {Vector3(0.5f, -0.5f, 0.5f),   Vector3(0.0f, 0.0f, 1.0f),  Color4(0.0f, 1.0f, 0.0f, 1.0f), Vector2(
    //                            1.0f, 1.0f)},
    //                    {Vector3(-0.5f, -0.5f, 0.5f),  Vector3(0.0f, 0.0f, 1.0f),  Color4(0.0f, 1.0f, 0.0f, 1.0f), Vector2(
    //                            0.0f, 1.0f)},
    //
    //                    // Right Face (X = -0.5, normal pointing left)
    //                    {Vector3(-0.5f, 0.5f, 0.5f),   Vector3(-1.0f, 0.0f, 0.0f), Color4(1.0f, 0.0f, 1.0f, 1.0f), Vector2(
    //                            0.0f, 0.0f)},
    //                    {Vector3(-0.5f, 0.5f, -0.5f),  Vector3(-1.0f, 0.0f, 0.0f), Color4(1.0f, 0.0f, 1.0f, 1.0f), Vector2(
    //                            1.0f, 0.0f)},
    //                    {Vector3(0.5f, 0.5f, 0.5f),    Vector3(-1.0f, 0.0f, 0.0f), Color4(1.0f, 0.0f, 1.0f, 1.0f), Vector2(
    //                            0.0f, 1.0f)},
    //                    {Vector3(0.5f, 0.5f, -0.5f),   Vector3(-1.0f, 0.0f, 0.0f), Color4(1.0f, 0.0f, 1.0f, 1.0f), Vector2(
    //                            1.0f, 1.0f)},
    //
    //                    // Left Face (X = 0.5, normal pointing right)
    //                    {Vector3(-0.5f, -0.5f, 0.5f),  Vector3(1.0f, 0.0f, 0.0f),  Color4(0.0f, 1.0f, 1.0f, 1.0f), Vector2(
    //                            0.0f, 0.0f)},
    //                    {Vector3(-0.5f, -0.5f, -0.5f), Vector3(1.0f, 0.0f, 0.0f),  Color4(0.0f, 1.0f, 1.0f, 1.0f), Vector2(
    //                            1.0f, 0.0f)},
    //                    {Vector3(0.5f, -0.5f, 0.5f),   Vector3(1.0f, 0.0f, 0.0f),  Color4(0.0f, 1.0f, 1.0f, 1.0f), Vector2(
    //                            0.0f, 1.0f)},
    //                    {Vector3(0.5f, -0.5f, -0.5f),  Vector3(1.0f, 0.0f, 0.0f),  Color4(0.0f, 1.0f, 1.0f, 1.0f), Vector2(
    //                            1.0f, 1.0f)},
    //
    //                    // Front Face (Y = 0.5, normal pointing up)
    //                    {Vector3(-0.5f, 0.5f, 0.5f),   Vector3(0.0f, 1.0f, 0.0f),  Color4(0.0f, 0.0f, 1.0f, 1.0f), Vector2(
    //                            0.0f, 0.0f)},
    //                    {Vector3(-0.5f, 0.5f, -0.5f),  Vector3(0.0f, 1.0f, 0.0f),  Color4(0.0f, 0.0f, 1.0f, 1.0f), Vector2(
    //                            1.0f, 0.0f)},
    //                    {Vector3(-0.5f, -0.5f, 0.5f),  Vector3(0.0f, 1.0f, 0.0f),  Color4(0.0f, 0.0f, 1.0f, 1.0f), Vector2(
    //                            0.0f, 1.0f)},
    //                    {Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.0f, 1.0f, 0.0f),  Color4(0.0f, 0.0f, 1.0f, 1.0f), Vector2(
    //                            1.0f, 1.0f)},
    //
    //                    // Back Face (Y = -0.5, normal pointing down)
    //                    {Vector3(0.5f, 0.5f, 0.5f),    Vector3(0.0f, -1.0f, 0.0f), Color4(1.0f, 1.0f, 0.0f, 1.0f), Vector2(
    //                            0.0f, 0.0f)},
    //                    {Vector3(0.5f, 0.5f, -0.5f),   Vector3(0.0f, -1.0f, 0.0f), Color4(1.0f, 1.0f, 0.0f, 1.0f), Vector2(
    //                            1.0f, 0.0f)},
    //                    {Vector3(0.5f, -0.5f, 0.5f),   Vector3(0.0f, -1.0f, 0.0f), Color4(1.0f, 1.0f, 0.0f, 1.0f), Vector2(
    //                            0.0f, 1.0f)},
    //                    {Vector3(0.5f, -0.5f, -0.5f),  Vector3(0.0f, -1.0f, 0.0f), Color4(1.0f, 1.0f, 0.0f, 1.0f), Vector2(
    //                            1.0f, 1.0f)},
    //            };
    //    static uint16_t cubeIndex[] =
    //            {
    //                    2, 1, 0,
    //                    3, 2, 0,
    //                    4, 5, 6, // top face
    //                    4, 6, 7,
    //                    8, 11, 10, //right face
    //                    8, 9, 11,
    //                    12, 15, 13, // left face
    //                    12, 14, 15,
    //                    16, 19, 17, // front face
    //                    16, 18, 19,
    //                    20, 21, 23, // back face
    //                    20, 23, 22
    //            };
    //
    //    VertexBuffer *vbo = new VertexBuffer(m_renderer, cubeVertex, sizeof(cubeVertex), cubeIndex, ARRAY_SIZE(cubeIndex),
    //                                         sizeof(uint16_t));
    //    m_mesh = new Mesh(m_renderer, vbo, material);
}

TestCube::~TestCube()
{
    delete m_mesh;
}
