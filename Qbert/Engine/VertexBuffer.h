//
// Created by William Zhao on 2/2/26.
//

#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "Renderer.h"
#include "EngineMath.h"

class VertexBuffer
{
public:
    VertexBuffer(class Renderer *renderer, void *vertexData, uint32_t vertexDataSize, void *indexData,
                 uint32_t numIndex,
                 uint32_t indexStride);

    ~VertexBuffer();

    void Draw(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) const;

private:
    SDL_GPUBuffer *m_indexBuffer = nullptr;
    SDL_GPUIndexElementSize m_indexBufferElementSize;

    SDL_GPUBuffer *m_vertexBuffer = nullptr;
    class Renderer *m_renderer = nullptr;
    uint32_t m_indexCount;
};


#endif //VERTEXBUFFER_H
