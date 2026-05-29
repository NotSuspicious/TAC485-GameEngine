//
// Created by William Zhao on 5/9/26.
//

#include "QbertLevel.h"

#include "Game.h"
#include "QbertCube.h"
#include "QbertBall.h"
#include "RedBall.h"
#include "SnakeSphere.h"
#include "Snake.h"
#include "ParabolicCurve.h"
#include "SkinnedObj.h"
#include "AssetManager.h"
#include "Mesh.h"
#include "JobManager.h"
#include "Components/Player.h"

QbertLevel::QbertLevel(Vector3 primary, Vector3 secondary, Vector3 activated, Player *player) : m_player(player)
{
    Game *game = Game::Get();

    //Generate qbert cubes
    m_originalActivatedColor = activated;
    for (int level = 0; level < MAX_PYRAMID_LEVEL; level++)
    {
        Vector3 startPosition = Vector3(
            0.0,
            CUBE_SIZE * level,
            -CUBE_SIZE * level);
        for (int i = 0; i <= level; i++)
        {
            auto *cube = new QbertCube(Renderer::Get());
            Quaternion rotation = Quaternion::CreateFromEuler(Vector3(0.0, 0.0, 90.0));
            auto positionOffset = Vector3(
                -CUBE_SIZE * (level - static_cast<float>(i)),
                -CUBE_SIZE * (level - static_cast<float>(i)),
                0.0);
            cube->SetPosition(startPosition + positionOffset);
            cube->SetRotation(rotation);
            cube->SetScale(CUBE_SIZE);
            cube->UpdateModelToWorld();
            cube->SetColors(primary, secondary, activated);
            m_qbertCubes.push_back(cube);
            game->GetActors().push_back(cube);
        }
    }

    m_ballDelay = Math::Random(BALL_DELAY_MIN, BALL_DELAY_MAX);
}

QbertLevel::~QbertLevel()
{
    // Cubes and balls are managed by Game's actors list, not by this class
    m_qbertCubes.clear();
    m_balls.clear();
    m_ballsToRemove.clear();
    m_snake = nullptr;
}

/// Returns the cube at a given pyramid level and horizonal index. Returns nullptr if out of bounds.
/// @param level the current pyramid level. 0 is the top level, and n-1 is the bottom level.
/// @param index the cube on a given level, 0-indexed from left-to-right.
/// @returns a QbertCube
QbertCube *QbertLevel::GetQbertCube(int level, int index)
{
    if (index > level || level >= MAX_PYRAMID_LEVEL || index < 0)
        return nullptr;
    const int cubeIndex = static_cast<float>(level) / 2 * (1 + level) + index;
    return m_qbertCubes[cubeIndex];
}

/// Activates input cube
/// @returns true if all cubes are activated
bool QbertLevel::ActivateCube(QbertCube *cube)
{
    if (cube != nullptr && !cube->IsActivated())
    {
        cube->SetActivation(true);
        if (++activatedCubes >= m_qbertCubes.size())
        {
            return true;
        }
    }
    return false;
}

bool QbertLevel::AreAllCubesActivated() const
{
    return activatedCubes >= m_qbertCubes.size();
}


void QbertLevel::ResetLevel()
{
    activatedCubes = 0;
    for (auto cube: m_qbertCubes)
    {
        cube->m_constants.activatedColor = m_originalActivatedColor;
        cube->SetActivation(false);
    }
}

void QbertLevel::Update(float deltaTime)
{
    // Wait for any animation jobs queued during this frame's actor updates to
    // finish before we delete actors. Character::UpdateAnim queues Jobs that
    // hold raw pointers back to the Character; deleting the owning actor
    // before those jobs run would dereference freed memory.
    if (!m_ballsToRemove.empty() || !m_actorsToRemove.empty())
    {
        JobManager::Get()->WaitForJobs();
    }

    // Process any deferred ball removals first (safe here - outside actors loop)
    for (QbertBall *ball: m_ballsToRemove)
    {
        Game::Get()->RemoveActor(ball);
    }
    m_ballsToRemove.clear();

    for (Actor *actor: m_actorsToRemove)
    {
        Game::Get()->RemoveActor(actor);
    }
    m_actorsToRemove.clear();

    if (m_player->HasWon())
    {
        m_flashTimer += deltaTime;

        if (m_flashTimer >= 1.0f / FLASH_FREQUENCY)
        {
            m_flashTimer -= 1.0f / FLASH_FREQUENCY;
            m_flashColorIndex = (m_flashColorIndex + 1) % FLASH_COLOR_AMOUNT;
        }
        for (auto cube: m_qbertCubes)
        {
            cube->m_constants.activatedColor = COLORS[m_flashColorIndex];
        }
    }

    // Spawn new balls on a timer (capped at MAX_BALLS), but only when the
    // player is alive (not dying, falling, or in a win flourish).
    if (m_player->IsAlive())
    {
        m_spawnTimer += deltaTime;
        if (m_spawnTimer >= BALL_SPAWN_INTERVAL && static_cast<int>(m_balls.size()) < MAX_BALLS)
        {
            m_spawnTimer = 0.0f;
            SpawnBall();
        }
    }

    // Update each ball; remove ones that finished falling off the grid
    // or were marked for destruction (e.g. SnakeSphere hatching into a Snake).
    for (auto it = m_balls.begin(); it != m_balls.end();)
    {
        QbertBall *ball = *it;
        UpdateBall(ball, deltaTime);
        const bool fellOff = (ball->m_state == QbertBall::State::FALL && ball->m_timer >= BALL_FALL_TIME);
        if (fellOff || ball->m_destroyMe)
        {
            it = m_balls.erase(it);
            Game::Get()->RemoveActor(ball);
        } else
        {
            ++it;
        }
    }
}

void QbertLevel::SpawnBall()
{
    QbertBall *ball;
    // First ball spawned (when no snake/snakesphere is present) is the SnakeSphere
    if (!m_snakeSphereSpawned && m_snake == nullptr)
    {
        ball = new SnakeSphere(Renderer::Get(), this);
        m_snakeSphereSpawned = true;
    } else
    {
        ball = new RedBall(Renderer::Get(), this);
    }
    ball->m_pyramidLevel = 0;
    ball->m_pyramidIndex = 0;
    ball->m_currentCube = GetQbertCube(0, 0);
    // Start the ball 5 cubes above the top cube and let it fall onto the cube.
    Vector3 landPos = ball->m_currentCube->GetPosition() +
                      Vector3::UnitZ * (CUBE_SIZE / 2.0f + BALL_VERTICAL_OFFSET);
    ball->m_pos = landPos + Vector3::UnitZ * BALL_SPAWN_FALL_HEIGHT;
    ball->m_spawnStartZ = ball->m_pos.z;
    ball->m_state = QbertBall::State::SPAWN_FALL;
    ball->m_timer = 0.0f;
    ball->m_constants.modelToWorld = Matrix4::CreateScale(ball->GetScale())
                                     * Matrix4::CreateTranslation(ball->m_pos);

    m_balls.push_back(ball);
    Game::Get()->GetActors().push_back(ball);
}

void QbertLevel::UpdateBall(QbertBall *ball, float deltaTime)
{
    ball->m_timer += deltaTime;

    switch (ball->m_state)
    {
        case QbertBall::State::SPAWN_FALL:
        {
            if (ball->m_timer >= BALL_SPAWN_FALL_TIME)
            {
                // Land on the top cube and check collision exactly once.
                ball->m_pos = ball->m_currentCube->GetPosition() +
                              Vector3::UnitZ * (CUBE_SIZE / 2.0f + BALL_VERTICAL_OFFSET);
                ball->m_state = QbertBall::State::DELAY;
                ball->m_timer = 0.0f;
                ball->OnLand();
                CheckPlayerBallCollision();
            } else
            {
                const float t = ball->m_timer / BALL_SPAWN_FALL_TIME;
                const float landZ = ball->m_currentCube->GetPosition().z + CUBE_SIZE / 2.0f + BALL_VERTICAL_OFFSET;
                ball->m_pos.z = Math::Lerp(ball->m_spawnStartZ, landZ, t);
            }
            break;
        }
        case QbertBall::State::DELAY:
        {
            if (ball->m_timer >= m_ballDelay)
            {
                m_ballDelay = Math::Random(BALL_DELAY_MIN, BALL_DELAY_MAX);
                ball->m_timer = 0.0f;

                // Random left or right (always going down a tier)
                bool moveRight = (rand() % 2) == 0;
                int newLevel = ball->m_pyramidLevel + 1;
                int newIndex = moveRight ? ball->m_pyramidIndex + 1 : ball->m_pyramidIndex;

                // Match player's jumpDirection convention so fall trajectory works
                ball->m_jumpDirection = moveRight ? Vector2(0.0f, 1.0f) : Vector2(-1.0f, 0.0f);

                ball->m_prevCube = ball->m_currentCube;
                QbertCube *nextCube = GetQbertCube(newLevel, newIndex);
                ball->m_currentCube = nextCube;
                ball->m_pyramidLevel = newLevel;
                ball->m_pyramidIndex = newIndex;

                if (nextCube == nullptr)
                {
                    // Off the grid - start falling and despawn after fall time
                    ball->m_state = QbertBall::State::FALL;
                    ball->m_fallStartZ = ball->m_pos.z;
                } else
                {
                    ball->m_state = QbertBall::State::JUMP;
                }
            }
            break;
        }
        case QbertBall::State::JUMP:
        {
            if (ball->m_timer >= BALL_JUMP_TIME)
            {
                // Land on next cube
                ball->m_pos = ball->m_currentCube->GetPosition() + Vector3::UnitZ * (
                                  CUBE_SIZE / 2.0f + BALL_VERTICAL_OFFSET);
                ball->m_state = QbertBall::State::DELAY;
                ball->m_timer = 0.0f;

                // Notify subclass first (may convert ball to snake), then check collision
                ball->OnLand();
                CheckPlayerBallCollision();
            } else
            {
                const float t = ball->m_timer / BALL_JUMP_TIME;
                // Always falling down a tier so sample is reversed (matches player down-jump)
                const float sampleT = 1.0f - t;
                const float jumpHeight = ball->m_jumpCurve->Sample(sampleT);

                ball->m_pos.x = Math::Lerp(ball->m_prevCube->GetPosition().x,
                                           ball->m_currentCube->GetPosition().x, t);
                ball->m_pos.y = Math::Lerp(ball->m_prevCube->GetPosition().y,
                                           ball->m_currentCube->GetPosition().y, t);
                const float baseZ = ball->m_prevCube->GetPosition().z - CUBE_SIZE / 2.0f + BALL_VERTICAL_OFFSET;
                ball->m_pos.z = baseZ + jumpHeight;
            }
            break;
        }
        case QbertBall::State::FALL:
        {
            if (ball->m_timer < BALL_FALL_TIME)
            {
                float t = ball->m_timer / BALL_FALL_TIME;
                ball->m_pos.z = ball->m_fallStartZ + (-2000.0f) * t * t;
                t *= 5.0f;
                ball->m_pos.x = Math::Lerp(ball->m_prevCube->GetPosition().x,
                                           ball->m_prevCube->GetPosition().x +
                                           ball->m_jumpDirection.x * CUBE_SIZE, t);
                ball->m_pos.y = Math::Lerp(ball->m_prevCube->GetPosition().y,
                                           ball->m_prevCube->GetPosition().y +
                                           ball->m_jumpDirection.y * CUBE_SIZE, t);
            }
            break;
        }
        case QbertBall::State::FROZEN:
            break;
    }

    // Push position into modelToWorld for rendering
    ball->m_constants.modelToWorld = Matrix4::CreateScale(ball->GetScale())
                                     * Matrix4::CreateTranslation(ball->m_pos);
}

bool QbertLevel::CheckPlayerBallCollision()
{
    QbertCube *playerCube = m_player->GetCurrentCube();
    if (playerCube == nullptr)
        return false;
    for (QbertBall *ball: m_balls)
    {
        // Only landed (DELAY-state) balls can collide with the player.
        if (ball->m_state != QbertBall::State::DELAY)
            continue;
        if (ball->GetCurrentCube() == playerCube)
        {
            ball->Freeze();
            m_player->Die();
            return true;
        }
    }
    return false;
}

bool QbertLevel::CheckPlayerSnakeCollision()
{
    if (m_snake == nullptr || m_snake->IsFrozen())
        return false;
    QbertCube *playerCube = m_player->GetCurrentCube();
    if (playerCube == nullptr)
        return false;
    if (m_snake->GetCurrentCube() == playerCube)
    {
        m_player->Die();
        return true;
    }
    return false;
}

void QbertLevel::FreezeSnake()
{
    if (m_snake != nullptr)
        m_snake->Freeze();
}

void QbertLevel::FreezeSnakeSphere()
{
    for (QbertBall *ball: m_balls)
    {
        if (dynamic_cast<SnakeSphere *>(ball) != nullptr)
        {
            ball->Freeze();
        }
    }
}

void QbertLevel::DespawnAllBalls()
{
    // Defer the actual removal from m_actors - this may be called during the
    // actor update loop (e.g. from Player::ResetPlayer)
    for (QbertBall *ball: m_balls)
    {
        m_ballsToRemove.push_back(ball);
    }
    m_balls.clear();
    m_spawnTimer = 0.0f;

    if (m_snake != nullptr)
    {
        // Defer the actual removal from m_actors to the next QbertLevel::Update tick
        // so we don't mutate the actor list while it's being iterated.
        m_actorsToRemove.push_back(m_snake->GetActor());
        m_snake = nullptr;
    }
    m_snakeSphereSpawned = false;
}

void QbertLevel::HatchSnake(SnakeSphere *sphere)
{
    // Only allow one snake on the board at a time
    if (m_snake != nullptr)
        return;

    int level = sphere->m_pyramidLevel;
    int index = sphere->m_pyramidIndex;
    QbertCube *cube = sphere->m_currentCube;

    // Mark the snake sphere for removal - the update loop will erase it safely
    sphere->m_destroyMe = true;

    // Build the snake as a SkinnedObj actor with a Snake (Character) component.
    AssetManager &am = Game::Get()->GetAssetManager();
    Mesh *snakeMesh = am.LoadMesh("Assets/Meshes/Snake.itpmesh3");
    auto *snakeActor = new SkinnedObj(Renderer::Get(), snakeMesh);
    snakeActor->m_constants.modelToWorld = Matrix4::Identity;
    m_snake = new Snake(snakeActor, this);
    snakeActor->AddComponent(m_snake);
    m_snake->SetGridPos(level, index, cube);
    Game::Get()->GetActors().push_back(snakeActor);
}

const Vector3 QbertLevel::COLORS[QbertLevel::FLASH_COLOR_AMOUNT] =
{
    Vector3(0.93f, 0.20f, 0.20f),
    Vector3(0.20f, 0.93f, 0.35f),
    Vector3(0.22f, 0.45f, 0.95f),
    Vector3(0.95f, 0.86f, 0.20f),
    Vector3(0.86f, 0.24f, 0.93f),
    Vector3(0.20f, 0.88f, 0.88f),
    Vector3(0.98f, 0.55f, 0.18f),
    Vector3(0.70f, 0.70f, 0.70f)
};
