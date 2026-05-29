//
// Created by William Zhao on 4/8/26.
//

#include "CollisionBox.h"
#include "JsonUtil.h"
#include "Actor.h"

void CollisionBox::LoadProperties(const rapidjson::Value &properties)
{
    Component::LoadProperties(properties);
    GetVectorFromJSON(properties, "min", m_AABB.m_minCorner);
    GetVectorFromJSON(properties, "max", m_AABB.m_maxCorner);
}

const AABB CollisionBox::GetAABB() const
{
    AABB a = m_AABB;
    float scale = m_actor->m_constants.modelToWorld.GetScale().x;
    Vector3 translation = m_actor->m_constants.modelToWorld.GetTranslation();
    a.m_minCorner = a.m_minCorner * scale + translation;
    a.m_maxCorner = a.m_maxCorner * scale + translation;
    return a;
}

CollisionBox::CollisionBox(Actor *actor) : Component(actor)
{
}
