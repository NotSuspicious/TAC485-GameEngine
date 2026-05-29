//
// Created by William Zhao on 3/23/26.
//

#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include "Component.h"
#include "EngineMath.h"
#include "JobManager.h"

class Character : public Component
{
private:
    class Job : public JobManager::Job
    {
        friend Character;

    public:
        explicit Job(Character *character) : m_character(character)
        {
        }

    private:
        void DoIt() override;

        Character *m_character;
    };

public:
    Character(class SkinnedObj *actor);

    void LoadProperties(const rapidjson::Value &properties) override;

    bool IsAnimDone();

protected:
    class SkinnedObj *m_skinnedObj;
    class Skeleton *m_skeleton = nullptr;
    std::unordered_map<std::string, const class Animation *> m_animations;
    const class Animation *m_currentAnimation = nullptr;
    float m_animationTime = 0.0f;
    bool m_isAnimFinished = false;

    bool SetAnim(const std::string &animName);

    void UpdateAnim(float deltaTime);

    void Update(float deltaTime) override;

    std::vector<Matrix4> globalPose;
};


#endif //GAME_CHARACTER_H
