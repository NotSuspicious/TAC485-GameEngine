//
// Created by William Zhao on 4/8/26.
//

#ifndef GAME_COLLISIONBOX_H
#define GAME_COLLISIONBOX_H


#include "Component.h"
#include "Physics.h"

class CollisionBox : public Component
{
public:
    CollisionBox(class Actor *actor);

    void LoadProperties(const rapidjson::Value &properties) override;

    const AABB GetAABB() const;

private:
    AABB m_AABB;
};


#endif //GAME_COLLISIONBOX_H
