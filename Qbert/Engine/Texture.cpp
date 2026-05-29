#include "pch.h"
#include "Texture.h"
#include "Profiler.h"
#include "Renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb/stb_image.h>

Texture::Texture(Renderer *renderer)
    : m_renderer(renderer)
{
}

Texture::~Texture()
{
    Free();
}

/// Release all resources used by the Texture and reset it back to an empty status
/// This function is automatically called by the destructor
/// You can also call this function manually if you want to change the usage of the Texture
void Texture::Free()
{
    if (nullptr != m_texture)
    {
        auto device = Renderer::Get()->GetDevice();
        SDL_ReleaseGPUTexture(device, m_texture);
        m_texture = nullptr;
    }
    m_width = 0;
    m_height = 0;
}

/// Load the texture from a file
/// At this point, only png files are supported
/// At this point, only 4-channel (RGBA) textures are supported
/// @param fileName the name of the texture file to load ("Assets/Textures/Cube.png")
/// @return true on success or false on failure
bool Texture::Load(const char *fileName)
{
    PROFILE_SCOPE(TextureLoad);

    int channels = 0;

    // attempt to load the file
    unsigned char *image = stbi_load(fileName, &m_width, &m_height, &channels, 0);
    if (image == nullptr)
    {
        SDL_Log("Failed to load image %s: %s", fileName, stbi_failure_reason());
        return false;
    }
    if (channels != 4)
    {
        // verify that it is a 4-channel (RGBA) format
        SDL_Log("Only 4 channel textures supported");
        stbi_image_free(image);
        return false;
    }

    // Create the GPU texture
    auto device = Renderer::Get()->GetDevice();
    SDL_GPUTextureCreateInfo texInfo = {};
    texInfo.type = SDL_GPU_TEXTURETYPE_2D;
    texInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texInfo.width = m_width;
    texInfo.height = m_height;
    texInfo.layer_count_or_depth = 1;
    texInfo.num_levels = 1;
    texInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    m_texture = SDL_CreateGPUTexture(device, &texInfo);
    SDL_SetGPUTextureName(
        device,
        m_texture,
        fileName
    );

    // Upload the texture to the GPU
    SDL_GPUTransferBufferCreateInfo createInfo = {};
    createInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    createInfo.size = m_width * m_height * 4;
    SDL_GPUTransferBuffer *textureTransferBuffer = SDL_CreateGPUTransferBuffer(device, &createInfo);

    void *textureTransferPtr = SDL_MapGPUTransferBuffer(device, textureTransferBuffer, false);
    SDL_memcpy(textureTransferPtr, image, m_width * m_height * 4);
    SDL_UnmapGPUTransferBuffer(device, textureTransferBuffer);

    SDL_GPUCommandBuffer *uploadCmdBuf = SDL_AcquireGPUCommandBuffer(device);
    SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    SDL_GPUTextureTransferInfo transferInfo = {};
    transferInfo.transfer_buffer = textureTransferBuffer;
    SDL_GPUTextureRegion texRegion = {};
    texRegion.texture = m_texture;
    texRegion.w = m_width;
    texRegion.h = m_height;
    texRegion.d = 1;
    SDL_UploadToGPUTexture(copyPass, &transferInfo, &texRegion, false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
    SDL_ReleaseGPUTransferBuffer(device, textureTransferBuffer);

    // we're done with the CPU copy of the data now
    stbi_image_free(image);

    return true;
}

/*static*/
Texture *Texture::StaticLoad(const char *fileName, AssetManager *pManager)
{
    Texture * pTex = new Texture(Renderer::Get());
    if (false == pTex->Load(fileName))
    {
        delete pTex;
        return nullptr;
    }
    return pTex;
}

/// Set this texture to be active on the GPU on the specified texture slot
/// AKA bind this texture to a texture register
/// @param renderPass
/// @param slot which slot this texture is on (Renderer::TEXTURE_SLOT_DIFFUSE)
void Texture::SetActive(SDL_GPURenderPass *renderPass, int slot) const
{
    PROFILE_SCOPE(TextureSetActive);
    SDL_GPUTextureSamplerBinding binding = {};
    binding.texture = m_texture;
    binding.sampler = m_renderer->GetSampler();
    SDL_BindGPUFragmentSamplers(renderPass, slot, &binding, 1);
}

bool Texture::CreateRenderTarget(int width, int height, SDL_GPUTextureFormat format, const char *name)
{
    PROFILE_SCOPE(TextureCreateRenderTarget);

    // Free any existing texture resources
    Free();

    // Store the dimensions and format
    m_width = width;
    m_height = height;
    m_format = format;

    // Create the GPU render target texture
    auto device = Renderer::Get()->GetDevice();
    SDL_GPUTextureCreateInfo texInfo = {};
    texInfo.type = SDL_GPU_TEXTURETYPE_2D;
    texInfo.format = format;
    texInfo.width = width;
    texInfo.height = height;
    texInfo.layer_count_or_depth = 1;
    texInfo.num_levels = 1;
    // Allow texture to be used as both a color target and a sampler
    texInfo.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;

    m_texture = SDL_CreateGPUTexture(device, &texInfo);
    if (nullptr == m_texture)
    {
        SDL_Log("Failed to create render target texture: %s", name);
        m_width = 0;
        m_height = 0;
        return false;
    }

    SDL_SetGPUTextureName(device, m_texture, name);
    return true;
}

