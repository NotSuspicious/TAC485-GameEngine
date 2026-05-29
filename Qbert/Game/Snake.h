//
// Created by William Zhao on 5/10/26.
//

#ifndef GAME_SNAKE_H
#define GAME_SNAKE_H

#include "Components/Character.h"
#include "EngineMath.h"

class QbertCube;
class QbertLevel;
class ParabolicCurve;
class SkinnedObj;

class Snake : public Character
{
public:
    enum class State
    {
        IDLE,
        JUMP,
        LAND,
        ATTACK
    };

    Snake(SkinnedObj *skinnedObj, QbertLevel *level);

    ~Snake() override;

    void Update(float deltaTime) override;

    QbertCube *GetCurrentCube() const { return m_currentCube; }

    void SetGridPos(int level, int index, QbertCube *cube);

    void Freeze() { m_isFrozen = true; }

    void Unfreeze() { m_isFrozen = false; }

    bool IsFrozen() const { return m_isFrozen; }

private:
    void StartJump();

    void SetState(State newState);

    void ChangeState();

    void UpdateTransform();

    QbertLevel *m_level = nullptr;
    State m_state = State::IDLE;
    QbertCube *m_currentCube = nullptr;
    QbertCube *m_prevCube = nullptr;
    int m_pyramidLevel = 0;
    int m_pyramidIndex = 0;
    Vector3 m_pos = Vector3::Zero;
    Vector2 m_jumpDirection = Vector2::Zero;
    bool m_isJumpingUp = true;
    float m_heading = 0.0f;
    float m_jumpTimer = 0.0f;
    float m_idleTimer = 0.0f;
    bool m_isFrozen = false;
    ParabolicCurve *m_jumpCurve = nullptr;
};


#endif //GAME_SNAKE_H
