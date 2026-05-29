#pragma once

class AssetManager;

// This class encapsulates Textures for use as both resources and render targets
class Texture
{
public:
    Texture(class Renderer *renderer);

    ~Texture();

    void Free();

    bool Load(const char *fileName);

    static Texture *StaticLoad(const char *fileName, AssetManager *pManager);

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    SDL_GPUTextureFormat GetFormat() const { return m_format; }
    SDL_GPUTexture *GetTexture() { return m_texture; }

    bool CreateRenderTarget(int width, int height, SDL_GPUTextureFormat format,
                            const char *name);

    void SetActive(SDL_GPURenderPass *renderPass, int slot) const;

private:
    int m_width = 0;
    int m_height = 0;
    SDL_GPUTextureFormat m_format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    SDL_GPUTexture *m_texture = nullptr;
    class Renderer *m_renderer = nullptr;
};
