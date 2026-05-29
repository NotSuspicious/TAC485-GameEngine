//
// Created by William Zhao on 5/10/26.
//

#include "SnakeSphere.h"

#include "Game.h"
#include "AssetManager.h"
#include "Mesh.h"
#include "QbertLevel.h"

SnakeSphere::SnakeSphere(Renderer *renderer, QbertLevel *level)
    : QbertBall(renderer, level)
{
    m_mesh = Game::Get()->GetAssetManager().LoadMesh("Assets/Meshes/SnakeSphere.itpmesh3");
}

void SnakeSphere::OnLand()
{
    if (m_pyramidLevel == QbertLevel::MAX_PYRAMID_LEVEL - 1)
    {
        m_level->HatchSnake(this);
    }
}
