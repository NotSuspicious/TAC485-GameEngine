//
// Created by William Zhao on 3/21/26.
//

#include "PointLight.h"
#include "Actor.h"
#include "JsonUtil.h"
#include "Profiler.h"

PointLight::PointLight(Actor *actor, Lighting *lighting)
    : Component(actor), m_lighting(lighting)
{
    m_pointLightData = m_lighting->AllocateLight();
}

PointLight::~PointLight()
{
    if (m_lighting && m_pointLightData)
    {
        m_lighting->FreeLight(m_pointLightData);
        m_pointLightData = nullptr;
    }
}

void PointLight::LoadProperties(const rapidjson::Value &properties)
{
    Component::LoadProperties(properties);
    if (!m_pointLightData)
    {
        return;
    }
    GetVectorFromJSON(properties, "lightColor", m_pointLightData->lightColor);
    GetFloatFromJSON(properties, "innerRadius", m_pointLightData->innerRadius);
    GetFloatFromJSON(properties, "outerRadius", m_pointLightData->outerRadius);
}

void PointLight::Update(float deltaTime)
{
    PROFILE_SCOPE(PointLightUpdate);
    Component::Update(deltaTime);
    if (m_pointLightData)
    {
        m_pointLightData->position = m_actor->GetPosition();
    }
}
