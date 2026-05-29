//
// Created by William Zhao on 3/21/26.
//

#ifndef GAME_COMPONENT_H
#define GAME_COMPONENT_H


class Component
{
public:
    Component(class Actor *actor);

    virtual ~Component();

    virtual void LoadProperties(const rapidjson::Value &properties);

    virtual void Update(float deltaTime);

    class Actor *GetActor() const { return m_actor; }

protected:
    class Actor *m_actor;
};


#endif //GAME_COMPONENT_H
