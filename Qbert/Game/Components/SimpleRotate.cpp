//
// Created by William Zhao on 4/22/26.
//

#include "SimpleRotate.h"
#include "Actor.h"
#include "JsonUtil.h"
#include "EngineMath.h"

void SimpleRotate::LoadProperties(const rapidjson::Value &properties)
{
    Component::LoadProperties(properties);
    GetFloatFromJSON(properties, "speed", m_speed);
}

void SimpleRotate::Update(float deltaTime)
{
    Component::Update(deltaTime);
    m_angle += m_speed * deltaTime;
    Vector3 position = m_actor->m_constants.modelToWorld.GetTranslation();
    Vector3 scale = m_actor->m_constants.modelToWorld.GetScale();
    m_actor->m_constants.modelToWorld = Matrix4::CreateScale(scale) * Matrix4::CreateRotationZ(m_angle) *
                                        Matrix4::CreateTranslation(position);
}

SimpleRotate::SimpleRotate(Actor *owner) : Component(owner)
{
}
