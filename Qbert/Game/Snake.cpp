//
// Created by William Zhao on 5/10/26.
//

#include "Snake.h"

#include "Game.h"
#include "AssetManager.h"
#include "Mesh.h"
#include "ParabolicCurve.h"
#include "QbertCube.h"
#include "QbertLevel.h"
#include "SkinnedObj.h"
#include "Components/Player.h"

namespace
{
    constexpr float SNAKE_JUMP_TIME = 0.5f;
    constexpr float SNAKE_IDLE_TIME = 0.6f;
}

Snake::Snake(SkinnedObj *skinnedObj, QbertLevel *level)
    : Character(skinnedObj), m_level(level)
{
    m_jumpCurve = new ParabolicCurve(0.0f, 100.0f, -800.0f);

    AssetManager &am = Game::Get()->GetAssetManager();
    m_skeleton = am.LoadSkeleton("Assets/Anims/Snake.itpskel");
    m_animations["idle"] = am.LoadAnimation("Assets/Anims/Snake_Idle.itpanim2");
    m_animations["jump"] = am.LoadAnimation("Assets/Anims/Snake_Jump.itpanim2");
    m_animations["land"] = am.LoadAnimation("Assets/Anims/Snake_Land.itpanim2");
    m_animations["attack"] = am.LoadAnimation("Assets/Anims/Snake_Attack.itpanim2");
    SetAnim("idle");
}

Snake::~Snake()
{
    delete m_jumpCurve;
}

void Snake::SetGridPos(int level, int index, QbertCube *cube)
{
    m_pyramidLevel = level;
    m_pyramidIndex = index;
    m_currentCube = cube;
    m_prevCube = cube;
    m_pos = cube->GetPosition() + Vector3::UnitZ * (QbertLevel::CUBE_SIZE / 2.0f + QbertLevel::BALL_VERTICAL_OFFSET);
    UpdateTransform();
}

void Snake::SetState(State newState)
{
    m_state = newState;
    switch (newState)
    {
        case State::IDLE:
            SetAnim("idle");
            break;
        case State::JUMP:
            SetAnim("jump");
            break;
        case State::LAND:
            SetAnim("land");
            break;
        case State::ATTACK:
            SetAnim("attack");
            break;
    }
}

void Snake::StartJump()
{
    Player *player = m_level->GetPlayer();
    if (player == nullptr)
        return;

    int playerLevel = player->GetCurrentPyramidLevel();
    int playerIndex = player->GetCurrentPyramidIndex();

    // Direction conventions match Player::Update:
    //   up    : level-1, index unchanged    (jumpDir = (1, 0))
    //   down  : level+1, index unchanged    (jumpDir = (-1, 0))
    //   left  : level-1, index-1            (jumpDir = (0, -1))
    //   right : level+1, index+1            (jumpDir = (0, 1))
    int candidates[4][4] = {
        {m_pyramidLevel - 1, m_pyramidIndex,        1,  0},
        {m_pyramidLevel + 1, m_pyramidIndex,       -1,  0},
        {m_pyramidLevel - 1, m_pyramidIndex - 1,    0, -1},
        {m_pyramidLevel + 1, m_pyramidIndex + 1,    0,  1},
    };

    int bestIdx = -1;
    int bestDistSq = INT32_MAX;
    for (int i = 0; i < 4; ++i)
    {
        int newLevel = candidates[i][0];
        int newIndex = candidates[i][1];
        // Stay on the grid - never jump off
        if (newLevel < 0 || newLevel >= QbertLevel::MAX_PYRAMID_LEVEL)
            continue;
        if (newIndex < 0 || newIndex > newLevel)
            continue;
        int d1 = playerLevel - newLevel;
        int d2 = playerIndex - newIndex;
        int distSq = d1 * d1 + d2 * d2;
        if (distSq < bestDistSq)
        {
            bestDistSq = distSq;
            bestIdx = i;
        }
    }

    if (bestIdx == -1)
    {
        m_idleTimer = 0.0f;
        return;
    }

    int newLevel = candidates[bestIdx][0];
    int newIndex = candidates[bestIdx][1];
    QbertCube *next = m_level->GetQbertCube(newLevel, newIndex);
    if (next == nullptr)
    {
        m_idleTimer = 0.0f;
        return;
    }

    m_jumpDirection = Vector2(static_cast<float>(candidates[bestIdx][2]),
                              static_cast<float>(candidates[bestIdx][3]));
    m_isJumpingUp = (newLevel < m_pyramidLevel);
    m_heading = atan2f(m_jumpDirection.y, m_jumpDirection.x);
    m_prevCube = m_currentCube;
    m_currentCube = next;
    m_pyramidLevel = newLevel;
    m_pyramidIndex = newIndex;
    m_jumpTimer = 0.0f;
    SetState(State::JUMP);
}

void Snake::ChangeState()
{
    if (m_state == State::LAND && IsAnimDone())
    {
        SetState(State::ATTACK);
    } else if (m_state == State::ATTACK && IsAnimDone())
    {
        SetState(State::IDLE);
        m_idleTimer = 0.0f;
    } else if (m_state == State::IDLE && IsAnimDone())
    {
        SetAnim("idle");
    }
}

void Snake::Update(float deltaTime)
{
    // Freeze when the player is dead, falling, or winning
    if (m_isFrozen)
    {
        UpdateTransform();
        Character::Update(deltaTime);
        return;
    }

    switch (m_state)
    {
        case State::IDLE:
        {
            m_idleTimer += deltaTime;
            if (m_idleTimer >= SNAKE_IDLE_TIME)
            {
                m_idleTimer = 0.0f;
                StartJump();
            }
            break;
        }
        case State::JUMP:
        {
            m_jumpTimer += deltaTime;
            if (m_jumpTimer >= SNAKE_JUMP_TIME)
            {
                m_pos = m_currentCube->GetPosition() + Vector3::UnitZ * (
                            QbertLevel::CUBE_SIZE / 2.0f + QbertLevel::BALL_VERTICAL_OFFSET);
                SetState(State::LAND);
                m_level->CheckPlayerSnakeCollision();
            } else
            {
                const float t = m_jumpTimer / SNAKE_JUMP_TIME;
                const float sampleT = m_isJumpingUp ? t : (1.0f - t);
                const float jumpHeight = m_jumpCurve->Sample(sampleT);

                m_pos.x = Math::Lerp(m_prevCube->GetPosition().x,
                                     m_currentCube->GetPosition().x, t);
                m_pos.y = Math::Lerp(m_prevCube->GetPosition().y,
                                     m_currentCube->GetPosition().y, t);
                float baseZ;
                if (m_isJumpingUp)
                {
                    baseZ = m_currentCube->GetPosition().z - QbertLevel::CUBE_SIZE / 2.0f +
                            QbertLevel::BALL_VERTICAL_OFFSET;
                } else
                {
                    baseZ = m_prevCube->GetPosition().z - QbertLevel::CUBE_SIZE / 2.0f +
                            QbertLevel::BALL_VERTICAL_OFFSET;
                }
                m_pos.z = baseZ + jumpHeight;
            }
            break;
        }
        case State::LAND:
        case State::ATTACK:
            // Stationary while playing land / attack animations
            break;
    }

    UpdateTransform();
    ChangeState();
    Character::Update(deltaTime);
}

void Snake::UpdateTransform()
{
    Matrix4 mat = Matrix4::CreateRotationZ(m_heading) * Matrix4::CreateTranslation(m_pos);
    m_actor->m_constants.modelToWorld = mat;
}
