//
// Created by William Zhao on 5/10/26.
//

#ifndef GAME_QBERTBALL_H
#define GAME_QBERTBALL_H

#include "Actor.h"
#include "EngineMath.h"

class QbertLevel;
class QbertCube;
class ParabolicCurve;

class QbertBall : public Actor
{
public:
    enum class State
    {
        SPAWN_FALL,
        DELAY,
        JUMP,
        FALL,
        FROZEN
    };

    QbertBall(class Renderer *renderer, QbertLevel *level);

    ~QbertBall() override;

    QbertCube *GetCurrentCube() const { return m_currentCube; }

    void Freeze() { m_state = State::FROZEN; }

    // Called when the ball lands on a cube. Default implementation is empty.
    virtual void OnLand() {}

    State m_state = State::SPAWN_FALL;
    bool m_destroyMe = false;
    float m_timer = 0.0f;
    QbertCube *m_currentCube = nullptr;
    QbertCube *m_prevCube = nullptr;
    int m_pyramidLevel = 0;
    int m_pyramidIndex = 0;
    Vector3 m_pos = Vector3::Zero;
    Vector2 m_jumpDirection = Vector2::Zero;
    ParabolicCurve *m_jumpCurve = nullptr;
    QbertLevel *m_level = nullptr;
    float m_fallStartZ = 0.0f;
    float m_spawnStartZ = 0.0f;
};


#endif //GAME_QBERTBALL_H
