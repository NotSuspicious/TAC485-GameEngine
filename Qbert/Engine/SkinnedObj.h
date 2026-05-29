//
// Created by William Zhao on 3/23/26.
//

#ifndef GAME_SKINNEDOBJ_H
#define GAME_SKINNEDOBJ_H

#include "Actor.h"

class SkinnedObj : public Actor
{
public:
    const static int MAX_BONES = 80;

    SkinnedObj(class Renderer *renderer, const class Mesh *mesh);

    struct SkinConstants
    {
        Matrix4 skinMatrix[MAX_BONES];
    };

    void Draw(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) override;

    SkinConstants constants;
};


#endif //GAME_SKINNEDOBJ_H
