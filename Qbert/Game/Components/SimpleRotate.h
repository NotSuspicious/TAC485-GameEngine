//
// Created by William Zhao on 4/22/26.
//

#ifndef GAME_SIMPLEROTATE_H
#define GAME_SIMPLEROTATE_H

#include "Component.h"


class SimpleRotate : public Component
{
public:
    SimpleRotate(class Actor *owner);

    void LoadProperties(const rapidjson::Value &properties) override;

    void Update(float deltaTime) override;

private:
    float m_angle = 0.0f;
    float m_speed = 0.0f;
};


#endif //GAME_SIMPLEROTATE_H
