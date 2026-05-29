//
// Created by William Zhao on 2/2/26.
//

#include "Actor.h"

#include "VertexBuffer.h"
#include "Renderer.h"
#include "Material.h"
#include "Mesh.h"
#include "Component.h"
#include "Profiler.h"

Actor::~Actor()
{
    for (auto component: m_components)
    {
        delete component;
    }
}


void Actor::Draw(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass)
{
    SDL_PushGPUVertexUniformData(commandBuffer, Renderer::ConstantBuffer_Vertex::CONSTANT_VERTEX_RENDEROBJ,
                                 &m_constants, sizeof(Actor::Constants));
    if (m_mesh != nullptr)
        m_mesh->Draw(commandBuffer, renderPass);
}

Actor::Actor(Renderer *renderer, const Mesh *mesh)
    : m_mesh(mesh),
      m_renderer(renderer),
      m_position(Vector3(0.0f, 0.0f, 0.0f))
{
    m_constants.modelToWorld = Matrix4::CreateRotationZ(Math::ToRadians(45.0f));
}

void Actor::AddComponent(Component *pComp)
{
    m_components.push_back(pComp);
}

void Actor::Update(float deltaTime)
{
    for (auto component: m_components)
    {
        component->Update(deltaTime);
    }
}

const Vector3 &Actor::GetPosition() const
{
    return m_position;
}

void Actor::SetPosition(const Vector3 &mPosition)
{
    m_position = mPosition;
}

float Actor::GetScale() const
{
    return m_scale;
}

void Actor::SetScale(float scale)
{
    m_scale = scale;
}

const Quaternion &Actor::GetRotation() const
{
    return m_rotation;
}

void Actor::SetRotation(const Quaternion &rotation)
{
    m_rotation = rotation;
}

void Actor::UpdateModelToWorld()
{
    m_constants.modelToWorld = Matrix4::CreateScale(m_scale) * Matrix4::CreateFromQuaternion(m_rotation) *
                               Matrix4::CreateTranslation(m_position);
}
