//
// Created by William Zhao on 3/23/26.
//

#include "Character.h"
#include "Renderer.h"
#include "../Game/Game.h"
#include "JsonUtil.h"
#include "Animation.h"
#include "SkinnedObj.h"
#include "Skeleton.h"
#include "Profiler.h"

void Character::LoadProperties(const rapidjson::Value &properties)
{
    Game *game = Renderer::Get()->GetGame();
    AssetManager &assetManager = game->GetAssetManager();
    m_skeleton = assetManager.LoadSkeleton(properties["skeleton"].GetString());
    auto &animations = properties["animations"];
    for (rapidjson::SizeType j = 0; j < animations.Size(); j++)
    {
        std::string animName = animations[j][0].GetString();
        Animation *pAnim = assetManager.LoadAnimation(animations[j][1].GetString());
        m_animations[animations[j][0].GetString()] = pAnim;
    }
}

bool Character::SetAnim(const std::string &animName)
{
    if (m_animations.find(animName) == m_animations.end())
    {
        return false;
    }
    m_currentAnimation = m_animations[animName];
    m_animationTime = 0.0f;
    m_isAnimFinished = false;
    return true;
}

void Character::UpdateAnim(float deltaTime)
{
    PROFILE_SCOPE(CharacterUpdateAnim);
    if (!m_currentAnimation)
    {
        return;
    }

    m_animationTime += deltaTime;
    if (!m_isAnimFinished)
    {
        if (m_animationTime >= m_currentAnimation->GetLength())
        {
            m_isAnimFinished = true;
        }
    }
    if (m_currentAnimation->IsLooping())
    {
        m_animationTime = fmod(m_animationTime, m_currentAnimation->GetLength());
    } else
    {
        m_animationTime = Math::Min(m_animationTime, m_currentAnimation->GetLength());
    }

    JobManager::Get()->AddJob(new Character::Job(this));
}

void Character::Update(float deltaTime)
{
    PROFILE_SCOPE(CharacterComponentUpdate);
    if (m_currentAnimation == nullptr)
    {
        SetAnim("run");
    }
    UpdateAnim(deltaTime);
}

Character::Character(SkinnedObj *actor) : Component(actor), m_skinnedObj(actor)
{
}

bool Character::IsAnimDone()
{
    return m_isAnimFinished;
}

void Character::Job::DoIt()
{
    PROFILE_SCOPE(CharacterJobAnimation);
    // Get the global pose at the current animation time
    m_character->globalPose.resize(m_character->m_skeleton->GetNumBones());
    m_character->m_currentAnimation->GetGlobalPoseAtTime(m_character->globalPose, m_character->m_skeleton,
                                                         m_character->m_animationTime);

    // Combine each global pose with its inverse bind pose
    const std::vector<Matrix4> &invBindPoses = m_character->m_skeleton->GetGlobalInvBindPose();
    for (size_t i = 0; i < m_character->m_skeleton->GetNumBones(); ++i)
    {
        m_character->m_skinnedObj->constants.skinMatrix[i] = invBindPoses[i] * m_character->globalPose[i];
    }
}

