#include "pch.h"
#include "Renderer.h"
#include "Profiler.h"
#include "../Game/Game.h"

/*static*/
Renderer *Renderer::s_theRenderer = nullptr;

Renderer::Renderer()
{
    DbgAssert(nullptr == s_theRenderer, "You can only have 1 Renderer");
    s_theRenderer = this;
}

Renderer::~Renderer()
{
    DbgAssert(this == s_theRenderer, "There can only be 1 Renderer");
    s_theRenderer = nullptr;
}

/// 1-time initialization of the Renderer
/// @param width Width of the window in pixels
/// @param height Height of the window in pixels
/// @return true on success or false on failure
bool Renderer::Init(int width, int height)
{
    PROFILE_SCOPE(RendererInit);

    // initialize SDL
    m_screenWidth = width;
    m_screenHeight = height;
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return false;
    }

    // create a window
    m_window = SDL_CreateWindow("Game", m_screenWidth, m_screenHeight, 0);

    // create the device
    m_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL, true, nullptr);
    bool ok = SDL_ClaimWindowForGPUDevice(m_device, m_window);

    SDL_GPUTextureFormat formats[5] = {
        SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
        SDL_GPU_TEXTUREFORMAT_D24_UNORM,
        SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
        SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        SDL_GPU_TEXTUREFORMAT_D16_UNORM
    };

    for (auto &format: formats)
    {
        if (SDL_GPUTextureSupportsFormat(m_device, format, SDL_GPU_TEXTURETYPE_2D,
                                         SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET))
        {
            m_textureFormat = format;
            break;
        }
    }

    // Make a default sampler
    SDL_GPUSamplerCreateInfo samplerInfo = {};
    samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    samplerInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.enable_anisotropy = true;
    samplerInfo.max_anisotropy = 4;
    m_sampler = SDL_CreateGPUSampler(m_device, &samplerInfo);

    // Create a depth/stencil texture to use as a depth target in render passes.
    SDL_GPUTextureCreateInfo textureInfo = {};
    textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
    textureInfo.format = m_textureFormat;
    textureInfo.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    textureInfo.width = static_cast<uint32_t>(m_screenWidth);
    textureInfo.height = static_cast<uint32_t>(m_screenHeight);
    textureInfo.layer_count_or_depth = 1;
    textureInfo.num_levels = 1;

    // Create and keep the depth texture. Some platforms/backends may require this explicitly.
    m_depthBuffer = SDL_CreateGPUTexture(m_device, &textureInfo);

    return ok;
}

/// Release all resources
/// This is called explicitly BEFORE the destructor to allow us to check for leaks
void Renderer::Shutdown()
{
    if (m_depthBuffer)
    {
        SDL_ReleaseGPUTexture(m_device, m_depthBuffer);
        m_depthBuffer = nullptr;
    }

    SDL_ReleaseGPUSampler(m_device, m_sampler);

    // destroy the GPU device
    SDL_DestroyGPUDevice(m_device);

    // destroy the window
    SDL_DestroyWindow(m_window);
}

/// Create a GPU buffer
/// These can be used for Vertex Buffers or Index Buffers
/// Be sure to release any buffers created with this function using SDL_ReleaseGPUBuffer
/// @param data the raw data to be sent to the GPU buffer
/// @param dataSize the size in bytes of the data
/// @param usage SDL_GPUBufferUsageFlags for the type of data (SDL_GPU_BUFFERUSAGE_VERTEX)
/// @return The SDL_GPUBuffer created
SDL_GPUBuffer *Renderer::CreateBuffer(void *data, uint32_t dataSize, SDL_GPUBufferUsageFlags usage)
{
    SDL_GPUBufferCreateInfo bufferInfo = {};
    bufferInfo.size = dataSize;
    bufferInfo.usage = usage;
    SDL_GPUBuffer *buffer = SDL_CreateGPUBuffer(m_device, &bufferInfo);
    if (buffer)
        UploadBuffer(buffer, data, dataSize);
    return buffer;
}

/// CreateBuffer automatically calls UploadBuffer, but if you change the data, you'll need to manually call
/// this function again
/// @param buffer the buffer to upload
/// @param data the raw data to be uploaded
/// @param dataSize the size in bytes of the data to be uploaded
void Renderer::UploadBuffer(SDL_GPUBuffer *buffer, void *data, uint32_t dataSize)
{
    PROFILE_SCOPE(RendererUploadBuffer);
    // create a transfer buffer to upload to the vertex buffer
    SDL_GPUTransferBufferCreateInfo transferInfo = {};
    transferInfo.size = dataSize;
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    SDL_GPUTransferBuffer *transferBuffer = SDL_CreateGPUTransferBuffer(m_device, &transferInfo); {
        // map the transfer buffer to a pointer
        void *map = SDL_MapGPUTransferBuffer(m_device, transferBuffer, false);
        memcpy(map, data, dataSize);
        // unmap the pointer when you are done updating the transfer buffer
        SDL_UnmapGPUTransferBuffer(m_device, transferBuffer);
    } {
        // Upload the transfer data to the vertex buffer
        SDL_GPUCommandBuffer *uploadCmdBuf = SDL_AcquireGPUCommandBuffer(m_device);
        SDL_GPUCopyPass *copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
        SDL_GPUTransferBufferLocation bufferLoc = {transferBuffer, 0};
        SDL_GPUBufferRegion bufferReg = {buffer, 0, dataSize};
        SDL_UploadToGPUBuffer(copyPass, &bufferLoc, &bufferReg, false);
        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
    }

    // we're done with the transfer
    SDL_ReleaseGPUTransferBuffer(m_device, transferBuffer);
}

/// Create a command buffer
/// The command buffer is used to send commands to the GPU
/// You must call BeginCommandBuffer() before you start rendering and call EndCommandBuffer() when you're done with the frame
/// @return the SDL_GPUCommandBuffer
SDL_GPUCommandBuffer *Renderer::BeginCommandBuffer()
{
    // acquire the command buffer
    SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(m_device); {
        // get the swapchain texture
        uint32_t width, height;
        SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, m_window, &m_backBuffer, &width, &height);
    }

    // end the frame early if a swapchain texture is not available
    if (m_backBuffer == nullptr)
    {
        // you must always submit the command buffer
        SDL_SubmitGPUCommandBuffer(commandBuffer);
        return nullptr;
    }
    return commandBuffer;
}

/// When you're done adding commands to the buffer, call this function to send the commands to the GPU
/// @param commandBuffer the SDL_GPUCommandBuffer you got from BeginCommandBuffer()
void Renderer::EndCommandBuffer(SDL_GPUCommandBuffer *commandBuffer)
{
    PROFILE_SCOPE(RendererEndCommandBuffer);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
    m_backBuffer = nullptr;
}

/// Clear the screen to the specified color
/// This creates a new SDL_GPURenderPass. You'll need to call EndRenderPass() when you're done with that screen/pass.
/// @param commandBuffer the SDL_GPUCommandBuffer you got from BeginCommandBuffer()
/// @param color
/// @return the SDL_GPURenderPass created
SDL_GPURenderPass *Renderer::ClearScreen(SDL_GPUCommandBuffer *commandBuffer, const SDL_FColor &color,
                                         bool useCustomBuffer)
{
    // create the color target
    SDL_GPUColorTargetInfo colorTargetInfo = {};
    colorTargetInfo.clear_color = color;
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    if (useCustomBuffer)
    {
        colorTargetInfo.texture = m_game->GetOffscreenTarget(Game::FULLSIZED)->GetTexture();
    } else
    {
        colorTargetInfo.texture = GetBackBuffer();
    }

    // begin a render pass
    SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo = {};
    depthStencilTargetInfo.texture = m_depthBuffer;
    depthStencilTargetInfo.cycle = true;
    depthStencilTargetInfo.clear_depth = 1.0f;
    depthStencilTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    depthStencilTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    depthStencilTargetInfo.stencil_load_op = SDL_GPU_LOADOP_CLEAR;
    depthStencilTargetInfo.stencil_store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, &depthStencilTargetInfo);
    return renderPass;
}

/// When you're finished with the render pass, call this to end it
/// @param renderPass the SDL_GPURenderPass you got from ClearScreen
void Renderer::EndRenderPass(SDL_GPURenderPass *renderPass)
{
    PROFILE_SCOPE(RendererEndRenderPass);
    SDL_EndGPURenderPass(renderPass);
}

SDL_GPURenderPass *Renderer::BeginRenderPass(SDL_GPUCommandBuffer *commandBuffer, const SDL_FColor &color,
                                             bool useCustomBuffer)
{
    //color target
    SDL_GPUColorTargetInfo colorTargetInfo = {};
    colorTargetInfo.clear_color = color;
    colorTargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    if (useCustomBuffer)
    {
        colorTargetInfo.texture = m_game->GetOffscreenTarget(Game::OffscreenTarget::FULLSIZED)->GetTexture();
    } else
    {
        colorTargetInfo.texture = GetBackBuffer();
    }

    SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);
    return renderPass;
}

/// Render to an arbitrary off-screen texture target
SDL_GPURenderPass *Renderer::BeginRenderPass(SDL_GPUCommandBuffer *commandBuffer, class Texture *colorTarget,
                                             const SDL_FColor &color)
{
    SDL_GPUColorTargetInfo colorTargetInfo = {};
    colorTargetInfo.clear_color = color;
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.texture = colorTarget->GetTexture();

    SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);
    return renderPass;
}

