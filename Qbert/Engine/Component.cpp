//
// Created by William Zhao on 3/21/26.
//

#include "Component.h"
#include "Actor.h"

Component::Component(Actor *actor)
    : m_actor(actor)
{
}

Component::~Component() = default;

void Component::LoadProperties(const rapidjson::Value &properties)
{
}

void Component::Update(float deltaTime)
{
}
