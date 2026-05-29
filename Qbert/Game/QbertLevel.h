//
// Created by William Zhao on 5/9/26.
//

#ifndef GAME_QBERTLEVEL_H
#define GAME_QBERTLEVEL_H
#include "EngineMath.h"


class QbertCube;
class QbertBall;
class Snake;
class SnakeSphere;

class QbertLevel
{
public:
    static constexpr float CUBE_SIZE = 100.0f;
    static constexpr int MAX_PYRAMID_LEVEL = 7;
    static constexpr float FLASH_FREQUENCY = 4.0f;
    static constexpr int FLASH_COLOR_AMOUNT = 8;
    static constexpr int MAX_BALLS = 2;
    static constexpr float BALL_SPAWN_INTERVAL = 3.0f;
    static constexpr float BALL_DELAY_MIN = .4f;
    static constexpr float BALL_DELAY_MAX = 2.0f;
    static constexpr float BALL_JUMP_TIME = 0.5f;
    static constexpr float BALL_FALL_TIME = 1.5f;
    static constexpr float BALL_VERTICAL_OFFSET = 25.0f;
    static constexpr float BALL_SPAWN_FALL_TIME = 0.6f;
    static constexpr float BALL_SPAWN_FALL_HEIGHT = 5.0f * CUBE_SIZE;
    static const Vector3 COLORS[FLASH_COLOR_AMOUNT];

    QbertLevel(Vector3 primary, Vector3 secondary, Vector3 activated, class Player *player);

    ~QbertLevel();

    QbertCube *GetQbertCube(int level, int index);

    bool ActivateCube(QbertCube *cube);

    bool AreAllCubesActivated() const;

    void ResetLevel();

    void Update(float deltaTime);

    bool CheckPlayerBallCollision();

    bool CheckPlayerSnakeCollision();

    void FreezeSnake();

    void FreezeSnakeSphere();

    void DespawnAllBalls();

    void HatchSnake(SnakeSphere *sphere);

    class Player *GetPlayer() const { return m_player; }

private:
    void SpawnBall();

    void UpdateBall(QbertBall *ball, float deltaTime);

    std::vector<QbertCube *> m_qbertCubes;
    int activatedCubes = 0;
    class Player *m_player = nullptr;
    float m_flashTimer = 0.0f;
    Vector3 m_originalActivatedColor;
    int m_flashColorIndex = 0;

    std::vector<QbertBall *> m_balls;
    std::vector<QbertBall *> m_ballsToRemove;
    std::vector<class Actor *> m_actorsToRemove;
    Snake *m_snake = nullptr;
    bool m_snakeSphereSpawned = false;
    float m_spawnTimer = 0.0f;
    float m_ballDelay;
};


#endif //GAME_QBERTLEVEL_H
