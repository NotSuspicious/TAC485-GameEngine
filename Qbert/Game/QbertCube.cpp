//
// Created by William Zhao on 4/30/26.
//

#include "QbertCube.h"

#include "Game.h"
#include "Material.h"
#include "Renderer.h"
#include "Mesh.h"
#include "VertexBuffer.h"
#include "VertexFormat.h"

// ...existing includes above...

QbertCube::QbertCube(Renderer *renderer)
    : Actor(renderer, nullptr)
{
    // Define vertices with per-face normals (vertices are duplicated per face)
    // This ensures each face has a constant normal and no interpolation artifacts
    static VertexData cubeVertex[] =
    {
        // Front face (z = +0.5) - vertices 0-3, normal = +Z
        {Vector3(-0.5f, -0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 1.0f)}, // 0
        {Vector3(-0.5f, 0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 0.0f)}, // 1
        {Vector3(0.5f, -0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f)}, // 2
        {Vector3(0.5f, 0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 0.0f)}, // 3

        // Top face (y = +0.5) - vertices 4-7, normal = +Y
        {Vector3(-0.5f, 0.5f, 0.5f), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)}, // 4 (copy of front-top-left)
        {Vector3(0.5f, 0.5f, 0.5f), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f)}, // 5 (copy of front-top-right)
        {Vector3(-0.5f, 0.5f, -0.5f), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f)}, // 6 (back-top-left)
        {Vector3(0.5f, 0.5f, -0.5f), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f)}, // 7 (back-top-right)

        // Right face (x = +0.5) - vertices 8-11, normal = +X
        {Vector3(0.5f, -0.5f, 0.5f), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f)}, // 8 (copy of front-bottom-right)
        {Vector3(0.5f, 0.5f, 0.5f), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)}, // 9 (copy of front-top-right)
        {Vector3(0.5f, 0.5f, -0.5f), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)}, // 10 (copy of back-top-right)
        {Vector3(0.5f, -0.5f, -0.5f), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)} // 11 (back-bottom-right)
    };

    // Create indices for front, top and right faces (each vertex is now unique per face)
    static uint16_t cubeIndex[] =
    {
        // Front face (0,1,3,2) - CCW from front
        0, 1, 3,
        0, 3, 2,

        // Top face (4,7,5,6) - CCW from top (looking down)
        4, 7, 5,
        4, 6, 7,

        // Right face (8,11,9,10) - CCW from right
        9, 11, 8,
        10, 11, 9
    };

    VertexBuffer *vbo = new VertexBuffer(m_renderer, (void *) cubeVertex, sizeof(cubeVertex), (void *) cubeIndex,
                                         (uint32_t) (sizeof(cubeIndex) / sizeof(cubeIndex[0])), sizeof(uint16_t));

    // Create a simple material using the Phong shader if available
    // Material *material = new Material();

    auto material = Game::Get()->GetAssetManager().LoadMaterial("Assets/Materials/QbertCube.itpmat");
    // if (m_renderer->GetGame())
    // {
    //     Shader *s = m_renderer->GetGame()->GetAssetManager().GetShader("Phong");
    //     if (s)
    //         material->SetShader(s);
    //     p
    // }
    // material->SetDiffuseColor(Vector3(1.0f, 0.6f, 0.2f));
    //
    m_mesh = new Mesh(m_renderer, vbo, material);
}

QbertCube::~QbertCube()
{
    delete m_mesh;
}

void QbertCube::Draw(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass)
{
    SetActive(commandBuffer);
    Actor::Draw(commandBuffer, renderPass);
}

void QbertCube::Update(float deltaTime)
{
    Actor::Update(deltaTime);
}

void QbertCube::SetActive(SDL_GPUCommandBuffer *command_buffer) const
{
    // Push Qbert constants to vertex shader (where they're used in the vertex shader)
    SDL_PushGPUVertexUniformData(command_buffer, Renderer::ConstantBuffer_Vertex::CONSTANT_VERTEX_QBERTCUBE,
                                 &m_constants, sizeof(QbertCube::Constants));
}

void QbertCube::SetColors(Vector3 primary, Vector3 secondary, Vector3 activated)
{
    m_constants.primaryColor = primary;
    m_constants.secondaryColor = secondary;
    m_constants.activatedColor = activated;
}
