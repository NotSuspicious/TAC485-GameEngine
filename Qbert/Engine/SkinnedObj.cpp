//
// Created by William Zhao on 3/23/26.
//

#include "SkinnedObj.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Profiler.h"

SkinnedObj::SkinnedObj(Renderer *renderer, const Mesh *mesh) : Actor(renderer, mesh)
{
    for (int i = 0; i < MAX_BONES; i++)
    {
        constants.skinMatrix[i] = Matrix4::Identity;
    }
}

void SkinnedObj::Draw(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass)
{
    PROFILE_SCOPE(SkinnedObjDraw);
    SDL_PushGPUVertexUniformData(commandBuffer, Renderer::ConstantBuffer_Vertex::CONSTANT_VERTEX_SKINNING,
                                 &constants, sizeof(SkinnedObj::SkinConstants));
    Actor::Draw(commandBuffer, renderPass);
}
