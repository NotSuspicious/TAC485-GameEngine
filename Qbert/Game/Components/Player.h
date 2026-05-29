#pragma once
#include "Curve.h"
#include "EngineMath.h"
#include "ParabolicCurve.h"
#include "Components/Character.h"

class Game;

class Player : public Character
{
public:
    Player(SkinnedObj *skinnedObj, Game *pGame);

    ~Player();

    void Update(float deltaTime) override;

    void Win();

    void Die();

    bool HasWon() const { return m_state == State::WIN; }

    bool IsAlive() const { return m_state != State::WIN && m_state != State::DIE && m_state != State::FALL; }

    void SetCurrentCube(class QbertCube *cube) { m_currentCube = cube; }

    class QbertCube *GetCurrentCube() const { return m_currentCube; }

    int GetCurrentPyramidLevel() const { return m_currentPyramidLevel; }

    int GetCurrentPyramidIndex() const { return m_currentPyramidIndex; }

protected:
    enum class State
    {
        UNKNOWN,
        IDLE,
        FALL,
        JUMP,
        LAND,
        WIN,
        DIE
    };

    void Jump(Vector2 move);

    void SetState(State newState);

    void ChangeState();

    void UpdateState(float deltaTime) const;

    void ResetPlayer();


    Game *m_game = nullptr;
    State m_state = State::UNKNOWN;
    float m_moveSpeed = 0.0f;
    float m_heading = 0.0f;
    Vector3 m_pos = Vector3::Zero;
    float m_prevAnimTime = 0.0f;

    float m_jumpTimer = 0.0f;
    class ParabolicCurve *m_jumpCurve = nullptr;
    Vector2 m_jumpDirection = Vector2::Zero;
    bool m_isJumpingUp = true; // true for W/A (up), false for S/D (down)

    float m_resetTimer = 0.0f;
    class ParabolicCurve *m_deathCurve = nullptr;
    float m_deathStartZ = 0.0f; // Z position when death starts


    int m_currentPyramidLevel = 0;
    int m_currentPyramidIndex = 0;
    class QbertCube *m_prevCube;
    class QbertCube *m_currentCube;

    int m_respawnPyramidLevel = 0;
    int m_respawnPyramidIndex = 0;
    bool m_cameraDetached = false;

    void SetCameraAttached(bool attached);
};
