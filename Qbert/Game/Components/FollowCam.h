#pragma once
#include "Component.h"
#include "EngineMath.h"

class Game;
class AssetManager;

class FollowCam : public Component
{
public:
    FollowCam(Actor *owner, Game *pGame);

    void LoadProperties(const rapidjson::Value &properties) override;

    void Update(float deltaTime) override;

    void SetAttached(bool attached) { m_attached = attached; }

    bool IsAttached() const { return m_attached; }

private:
    bool m_attached = true;
    Game *m_game = nullptr;
    // current location of the camera relative to the target (Player)
    float m_azimuth = 0.0f;
    float m_elevation = 0.0f;
    float m_radius = 0.0f;

    // control parameters
    Vector3 m_offset = Vector3::Zero;
    Vector3 mCenter = Vector3::Zero;
    float m_yawSpeed = 0.0f;
    float m_pitchSpeed = 0.0f;
    float m_elvMin = 0.0f;
    float m_elvMax = 0.0f;
};