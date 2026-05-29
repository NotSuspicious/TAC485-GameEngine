//
// Created by William Zhao on 5/10/26.
//

#include "RedBall.h"

#include "Game.h"
#include "AssetManager.h"
#include "Mesh.h"

RedBall::RedBall(Renderer *renderer, QbertLevel *level)
    : QbertBall(renderer, level)
{
    m_mesh = Game::Get()->GetAssetManager().LoadMesh("Assets/Meshes/RedSphere.itpmesh3");
}
