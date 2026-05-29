#include "pch.h"
#include "Components/Player.h"
#include "Components/FollowCam.h"
#include "Actor.h"
#include "Game.h"
#include "SkinnedObj.h"
#include "QbertCube.h"
#include "QbertLevel.h"


static constexpr float s_jumpTime = 0.5f;
static constexpr float s_deathTime = 3.0f;

Player::Player(SkinnedObj *skinnedObj, Game *pGame)
    : Character(skinnedObj)
      , m_game(pGame)
      , m_prevCube(nullptr)
{
    Matrix4 mat = m_actor->m_constants.modelToWorld;
    m_pos = mat.GetTranslation();
    Vector3 fwd = mat.GetXAxis();
    m_heading = atan2f(fwd.y, fwd.x);

    // Create jump curve: goes from 0 to 100 height offset
    m_jumpCurve = new ParabolicCurve(0.0f, 100.0f, -800.0f);

    // Create death curve: falls from current position to -2000
    m_deathCurve = new ParabolicCurve(0.0f, -2000.0f, -8000.81f);
}

Player::~Player()
{
    if (m_jumpCurve)
    {
        delete m_jumpCurve;
    }
    if (m_deathCurve)
    {
        delete m_deathCurve;
    }
}

void Player::Update(float deltaTime)
{
    if (m_state == State::WIN || m_state == State::DIE)
    {
        m_resetTimer += deltaTime;
        if (m_resetTimer >= s_deathTime)
        {
            if (m_state == State::WIN)
            {
                Game::Get()->GetQbertLevel()->ResetLevel();
            }
            ResetPlayer();
        }
    } else if (m_state != State::WIN && m_state != State::DIE && m_state != State::FALL)
    {
        // Read keyboard input (only when not jumping)
        m_jumpDirection = Vector2::Zero;
        if (m_state != State::JUMP)
        {
            if (m_game->IsKeyHit(SDL_SCANCODE_W) || m_game->IsKeyHit(SDL_SCANCODE_UP))
            {
                m_currentPyramidLevel--;
                m_jumpDirection = Vector2(1.0f, 0.0f);
                m_isJumpingUp = true;
            } else if (m_game->IsKeyHit(SDL_SCANCODE_S) || m_game->IsKeyHit(SDL_SCANCODE_DOWN))
            {
                m_currentPyramidLevel++;
                m_jumpDirection = Vector2(-1.0f, 0.0f);
                m_isJumpingUp = false;
            } else if (m_game->IsKeyHit(SDL_SCANCODE_A) || m_game->IsKeyHit(SDL_SCANCODE_LEFT))
            {
                m_currentPyramidLevel--;
                m_currentPyramidIndex--;
                m_jumpDirection = Vector2(0.0f, -1.0f);
                m_isJumpingUp = true;
            } else if (m_game->IsKeyHit(SDL_SCANCODE_D) || m_game->IsKeyHit(SDL_SCANCODE_RIGHT))
            {
                m_currentPyramidLevel++;
                m_currentPyramidIndex++;
                m_jumpDirection = Vector2(0.0f, 1.0f);
                m_isJumpingUp = false;
            }

            // Rotate toward move direction
            if (!Vector2::IsCloseEnuf(m_jumpDirection, Vector2::Zero))
            {
                m_heading = atan2f(m_jumpDirection.y, m_jumpDirection.x);
            }
        }

        // Handle jump completion (timer counts up)
        if (m_state == State::JUMP)
        {
            m_jumpTimer += deltaTime;
            if (m_jumpTimer >= s_jumpTime)
            {
                m_jumpTimer = 0.0f;
                m_pos = m_currentCube->GetPosition() + Vector3::UnitZ * QbertLevel::CUBE_SIZE / 2.0f;
                Game::Get()->GetQbertLevel()->ActivateCube(m_currentCube);
                SetState(State::LAND);
            } else
            {
                // Calculate normalized jump arc
                const float t = m_jumpTimer / s_jumpTime;

                // Sample based on jump direction
                float sampleT = m_isJumpingUp ? t : (1.0f - t);
                const float jumpHeight = m_jumpCurve->Sample(sampleT);

                // Linear interpolation for X and Y
                m_pos.x = Math::Lerp(m_prevCube->GetPosition().x, m_currentCube->GetPosition().x, t);
                m_pos.y = Math::Lerp(m_prevCube->GetPosition().y, m_currentCube->GetPosition().y, t);

                // Interpolate Z from previous cube to current cube, then add jump arc
                float baseZ;
                if (m_isJumpingUp)
                {
                    baseZ = m_currentCube->GetPosition().z - QbertLevel::CUBE_SIZE / 2.0f;
                } else baseZ = m_prevCube->GetPosition().z - QbertLevel::CUBE_SIZE / 2.0f;
                m_pos.z = baseZ + jumpHeight;
            }
        }

        // Start new jump if player moved
        if (m_state != State::JUMP && !Vector2::IsCloseEnuf(m_jumpDirection, Vector2::Zero))
        {
            m_prevCube = m_currentCube;
            m_currentCube = m_game->GetQbertLevel()->GetQbertCube(m_currentPyramidLevel, m_currentPyramidIndex);

            if (m_currentCube)
            {
                Jump(m_jumpDirection);
                SetState(State::JUMP);
            } else
            {
                // Player fell off - start death
                m_resetTimer = 0.0f;
                m_deathStartZ = m_pos.z;
                SetState(State::FALL);
            }
        }
    } else if (m_state == State::FALL)
    {
        // Handle death (timer counts up)
        m_resetTimer += deltaTime;
        if (m_resetTimer >= s_deathTime)
        {
            ResetPlayer();
        } else
        {
            // Fall to bottom using death curve
            float t = m_resetTimer / s_deathTime;
            const float deathHeight = m_deathCurve->Sample(t);
            m_pos.z = m_deathStartZ + deathHeight; // Linear interpolation for X and Y
            //Make the jump distance farther
            t *= 5.0f;
            m_pos.x = Math::Lerp(m_prevCube->GetPosition().x,
                                 m_prevCube->GetPosition().x + m_jumpDirection.x * QbertLevel::CUBE_SIZE, t);
            m_pos.y = Math::Lerp(m_prevCube->GetPosition().y,
                                 m_prevCube->GetPosition().y + m_jumpDirection.y * QbertLevel::CUBE_SIZE, t);

            // Detach the camera once the player has fallen 100f below where the fall began.
            if (!m_cameraDetached && (m_deathStartZ - m_pos.z) >= 100.0f)
            {
                SetCameraAttached(false);
                m_cameraDetached = true;
            }
        }
    }
    m_prevAnimTime = m_animationTime;
    Character::Update(deltaTime);

    ChangeState();
    UpdateState(deltaTime);
}

void Player::ResetPlayer()
{
    // Despawn balls and any active snake before respawning.
    Game::Get()->GetQbertLevel()->DespawnAllBalls();

    m_resetTimer = 0.0f;
    m_jumpTimer = 0.0f;
    m_currentPyramidLevel = m_respawnPyramidLevel;
    m_currentPyramidIndex = m_respawnPyramidIndex;
    m_currentCube = Game::Get()->GetQbertLevel()->GetQbertCube(m_currentPyramidLevel, m_currentPyramidIndex);
    m_prevCube = m_currentCube;
    m_pos = m_currentCube->GetPosition() + Vector3::UnitZ * QbertLevel::CUBE_SIZE / 2.0f;

    // After respawn, default the next respawn back to the top of the pyramid (used for falls)
    m_respawnPyramidLevel = 0;
    m_respawnPyramidIndex = 0;

    // Reattach the camera to the player.
    if (m_cameraDetached)
    {
        SetCameraAttached(true);
        m_cameraDetached = false;
    }

    m_state = State::UNKNOWN;
    SetState(State::IDLE);
}

void Player::Jump(Vector2 move)
{
    m_jumpTimer = 0.0f;
}

/// Set the state of the player to newState and start any animations that go with that
/// @param newState
void Player::SetState(State newState)
{
    if (m_state == State::WIN || m_state == State::DIE)
        return;
    switch (newState)
    {
        case State::UNKNOWN:
        case State::IDLE:
            SetAnim("idle");
            break;
        case State::FALL:
            SetAnim("fall");
            break;
        case State::WIN:
            SetAnim("win");
            break;
        case State::JUMP:
            SetAnim("jump");
            break;
        case State::LAND:
            SetAnim("land");
            break;
        case State::DIE:
            SetAnim("die");
            break;
        default:
            break;
    }
    m_state = newState;

    // Freeze the snake (and snake sphere) while the player is dead, falling, or has won.
    if (newState == State::DIE || newState == State::FALL || newState == State::WIN)
    {
        Game::Get()->GetQbertLevel()->FreezeSnake();
        Game::Get()->GetQbertLevel()->FreezeSnakeSphere();
    }

    // Check ball/snake collision after landing on a cube
    if (newState == State::LAND)
    {
        Game::Get()->GetQbertLevel()->CheckPlayerBallCollision();
        Game::Get()->GetQbertLevel()->CheckPlayerSnakeCollision();
    }
}

/// Check to see if the player needs to switch states and switch as necessary
void Player::ChangeState()
{
    switch (m_state)
    {
        case State::UNKNOWN:
            SetState(State::IDLE);
            break;
        case State::LAND:
            if (IsAnimDone())
                SetState(State::IDLE);
        // no break... fall through
        case State::IDLE:
            // Keep the idle animation looping by restarting it whenever it finishes.
            if (m_state == State::IDLE && IsAnimDone())
                SetAnim("idle");
            if (m_game->GetQbertLevel()->AreAllCubesActivated())
            {
                Win();
            }
            break;
        case State::JUMP:
            if (IsAnimDone())
                SetAnim("jump");
        // no break... fall through
        case State::FALL:
            break;
        default:
            break;
    }
}

/// Update the player's current state
void Player::UpdateState(float deltaTime) const
{
    Matrix4 mat = Matrix4::CreateRotationZ(m_heading);
    m_actor->m_constants.modelToWorld = mat
                                        * Matrix4::CreateTranslation(m_pos);
}

void Player::Win()
{
    m_resetTimer = 0.0f;
    SetState(State::WIN);
}

void Player::Die()
{
    m_resetTimer = 0.0f;
    // Respawn on the cube where the collision happened (not the top of the pyramid).
    m_respawnPyramidLevel = m_currentPyramidLevel;
    m_respawnPyramidIndex = m_currentPyramidIndex;
    SetState(State::DIE);
}

void Player::SetCameraAttached(bool attached)
{
    for (Component *comp: m_actor->GetComponents())
    {
        if (auto *cam = dynamic_cast<FollowCam *>(comp))
        {
            cam->SetAttached(attached);
            return;
        }
    }
}

