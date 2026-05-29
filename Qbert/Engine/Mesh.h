#pragma once

class AssetManager;

class Material;

class Renderer;

class VertexBuffer;

class Mesh
{
public:
    Mesh(Renderer *renderer, VertexBuffer *vertexBuffer, Material *material);

    ~Mesh();

    bool Load(void *vertexData, uint32_t vertexDataSize, void *indexData, uint32_t numIndex, uint32_t indexStride,
              Material *material);

    bool Load(const char *fileName, AssetManager *pAssetManager);

    static Mesh *StaticLoad(const char *fileName, AssetManager *pAssetManager);

    void Draw(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) const;

    bool IsSkinned() const { return m_isSkinned; }

protected:
    Renderer *m_renderer = nullptr;
    bool m_isSkinned = false;
    VertexBuffer *m_vertexBuffer = nullptr;
    Material *m_material = nullptr;
};
