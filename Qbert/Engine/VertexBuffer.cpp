//
// Created by William Zhao on 2/2/26.
//

#include "VertexBuffer.h"

VertexBuffer::VertexBuffer(class Renderer *renderer, void *vertexData, uint32_t vertexDataSize, void *indexData,
                           uint32_t numIndex,
                           uint32_t indexStride)
    : m_renderer(renderer)
      , m_indexCount(numIndex)
{
    m_vertexBuffer = m_renderer->CreateBuffer(vertexData, vertexDataSize, SDL_GPU_BUFFERUSAGE_VERTEX);
    m_indexBuffer = m_renderer->CreateBuffer(indexData, indexStride * numIndex, SDL_GPU_BUFFERUSAGE_INDEX);
    m_indexBufferElementSize = (indexStride <= 2) ? SDL_GPU_INDEXELEMENTSIZE_16BIT : SDL_GPU_INDEXELEMENTSIZE_32BIT;
}

VertexBuffer::~VertexBuffer()
{
    if (m_vertexBuffer)
    {
        SDL_ReleaseGPUBuffer(m_renderer->GetDevice(), m_vertexBuffer);
    }
    if (m_indexBuffer)
    {
        SDL_ReleaseGPUBuffer(m_renderer->GetDevice(), m_indexBuffer);
    }
}

void VertexBuffer::Draw(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) const
{
    SDL_GPUBufferBinding vertex_buffer_binding[1];
    vertex_buffer_binding[0].buffer = m_vertexBuffer;
    vertex_buffer_binding[0].offset = 0;

    SDL_GPUBufferBinding index_buffer_binding[1];
    index_buffer_binding[0].buffer = m_indexBuffer;
    index_buffer_binding[0].offset = 0;

    SDL_BindGPUVertexBuffers(renderPass, 0, vertex_buffer_binding, 1);
    SDL_BindGPUIndexBuffer(renderPass, index_buffer_binding, m_indexBufferElementSize);
    SDL_DrawGPUIndexedPrimitives(renderPass, m_indexCount, 1, 0, 0, 0);
}
