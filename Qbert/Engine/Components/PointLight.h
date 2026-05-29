//
// Created by William Zhao on 3/21/26.
//

#ifndef GAME_POINTLIGHT_H
#define GAME_POINTLIGHT_H

#include "Component.h"
#include "Lighting.h"

class PointLight : public Component
{
public:
    PointLight(class Actor *actor, Lighting *lighting);

    ~PointLight() override;

    void LoadProperties(const rapidjson::Value &properties) override;

    void Update(float deltaTime) override;

private:
    Lighting *m_lighting;
    Lighting::PointLightData *m_pointLightData;
};


#endif //GAME_POINTLIGHT_H
