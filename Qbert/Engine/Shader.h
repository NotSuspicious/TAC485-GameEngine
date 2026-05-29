#pragma once
#include <SDL3/SDL.h>

class Renderer;
class Texture;

enum class DepthFlags : uint8_t
{
    NONE = 0,
    TEST = 1 << 0, // 0x01
    WRITE = 1 << 1 // 0x02
};

inline DepthFlags operator|(DepthFlags a, DepthFlags b)
{
    return static_cast<DepthFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline DepthFlags operator&(DepthFlags a, DepthFlags b)
{
    return static_cast<DepthFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

// Shader encapsulates the Vertex Shader, the Fragment Shader, and the entire GPU Pipeline
// That means a Shader is set up to handle only 1 format of texture output, and a unique set of blend options
class Shader
{
public:
    Shader(Renderer *renderer, const char *filename);

    ~Shader();

    void SetColorTarget(Texture *colorTarget = nullptr);

    void SetDepthFlags(DepthFlags flags) { m_depthFlags = flags; }
    DepthFlags GetDepthFlags() const { return m_depthFlags; }
    bool IsZTestEnabled() const { return (m_depthFlags & DepthFlags::TEST) != DepthFlags::NONE; }
    bool IsZWriteEnabled() const { return (m_depthFlags & DepthFlags::WRITE) != DepthFlags::NONE; }

    // Blend state functions
    void SetBlendEnabled(bool enabled) { m_blendEnabled = enabled; }

    void SetBlendFactors(SDL_GPUBlendFactor src, SDL_GPUBlendFactor dst)
    {
        m_srcColorBlendFactor = src;
        m_dstColorBlendFactor = dst;
    }

    bool IsBlendEnabled() const { return m_blendEnabled; }
    SDL_GPUBlendFactor GetSrcColorBlendFactor() const { return m_srcColorBlendFactor; }
    SDL_GPUBlendFactor GetDstColorBlendFactor() const { return m_dstColorBlendFactor; }

    bool CreatePipeline(const SDL_GPUVertexAttribute *vertexAttributes, uint32_t numVertexAttributes,
                        uint32_t vertexStride);

    void SetActive(SDL_GPURenderPass *renderPass);

private:
    Renderer *m_renderer = nullptr;
    // we're only supporting a single color target
    Texture *m_colorTarget = nullptr;
    // shaders
    SDL_GPUShader *m_vertexShader = nullptr;
    SDL_GPUShader *m_fragmentShader = nullptr;
    // the shaders are combined into a pipeline
    SDL_GPUGraphicsPipeline *m_pipeline = nullptr;
    // ...existing code...
    // depth flags (default to TEST | WRITE enabled)
    DepthFlags m_depthFlags = DepthFlags::TEST | DepthFlags::WRITE;
    // blend state settings (default to disabled)
    bool m_blendEnabled = false;
    SDL_GPUBlendFactor m_srcColorBlendFactor = SDL_GPU_BLENDFACTOR_ONE;
    SDL_GPUBlendFactor m_dstColorBlendFactor = SDL_GPU_BLENDFACTOR_ZERO;

    bool Load(const char *filename);
};