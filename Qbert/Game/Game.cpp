#include "pch.h"
#include "Game.h"

#include "Actor.h"
#include "Camera.h"
#include "JsonUtil.h"
#include "Mesh.h"
#include "Profiler.h"
#include "JobManager.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexFormat.h"
#include "VertexBuffer.h"
#include "Lighting.h"
#include "AssetManager.h"
#include "QbertLevel.h"
#include "Components/PointLight.h"
#include "SkinnedObj.h"
#include "Components/Character.h"
#include "Components/Player.h"
#include "Components/FollowCam.h"
#include "Components/CollisionBox.h"
#include "Components/SimpleRotate.h"

Game *Game::s_instance = nullptr;

Game *Game::Get()
{
    if (s_instance == nullptr)
    {
        s_instance = new Game();
    }
    return s_instance;
}

Game::Game() : m_offscreenTargets{}, m_qbertLevel(nullptr)
{
}

Game::~Game()
{
}

/// One-time Initialization of the Game.
/// @param width the width of the window in pixels
/// @param height the height of the window in pixels
/// @return true on success or false on failure
bool Game::Init(int width, int height)
{
    PROFILE_SCOPE(GameInit)
    if (false == m_renderer.Init(800, 600))
    {
        DbgAssert(false, "Failed to init renderer");
        return false;
    }

    m_renderer.SetGame(this);

    for (int i = 0; i < OFFSCREEN_TARGET_COUNT; i++)
    {
        m_offscreenTargets[i] = new Texture(&m_renderer);
    }
    m_offscreenTargets[OffscreenTarget::FULLSIZED]->CreateRenderTarget(
        800, 600, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT, "post1");
    m_offscreenTargets[OffscreenTarget::HALF]->CreateRenderTarget(400, 300, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT,
                                                                  "post2");
    m_offscreenTargets[OffscreenTarget::QUARTER]->CreateRenderTarget(200, 150, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT,
                                                                     "post3");
    m_offscreenTargets[OffscreenTarget::QUARTER2]->CreateRenderTarget(
        200, 150, SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT, "post4");
    LoadShaders();

    m_lighting = new Lighting();
    m_camera = new Camera(&m_renderer);
    m_physics = new Physics();


    LoadLevel("Assets/Levels/Qbert.itplevel");

    VertexUV fullscreenQuad[]{
        {Vector3(-1.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f)},
        {Vector3(1.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f)},
        {Vector3(1.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f)},
        {Vector3(-1.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)}
    };
    int quadIndices[]{0, 1, 2, 0, 2, 3};
    m_fullscreenBuffer = new VertexBuffer(&m_renderer, fullscreenQuad, 4 * sizeof(VertexUV), quadIndices, 6,
                                          sizeof(int));
    m_ticksCount = SDL_GetTicks();

    JobManager::Get()->Begin();

    return true;
}

/// Shut down the game and release all resources
/// We do this in Shutdown BEFORE the destructor so that we can check for leaks
void Game::Shutdown()
{
    JobManager::Get()->End();

    delete[] m_lastKeyState;
    delete[] m_keyState;

    // release all assets
    delete m_qbertLevel;
    for (Actor *actor: m_actors)
    {
        delete actor;
    }
    for (int i = 0; i < OFFSCREEN_TARGET_COUNT; i++)
    {
        delete m_offscreenTargets[i];
    }
    delete m_fullscreenBuffer;
    delete m_physics;
    delete m_camera;
    delete m_lighting;
    m_assetManager.Clear();
}

/// Process the input for a single frame of gameplay
void Game::ProcessInput()
{
    PROFILE_SCOPE(GameProcessInput);
    // copy the keys from last frame to m_lastKeyState
    if (m_numKeys > 0)
        memcpy((void *) m_lastKeyState, (void *) m_keyState, m_numKeys * sizeof(bool));
    m_lastMouseButtons = m_mouseButtons;

    // read the current keys
    int numKeys = 0;
    const bool *keyState = SDL_GetKeyboardState(&numKeys);

    if (numKeys > m_numKeys)
    {
        // reallocate the array of keys (both current and last frame)
        if (m_keyState)
            delete[] m_keyState;
        if (m_lastKeyState)
            delete[] m_lastKeyState;
        m_keyState = new bool[numKeys]();
        m_lastKeyState = new bool[numKeys]();
    }

    // store the current frame's keys as m_keyState
    if (numKeys > 0)
        memcpy((void *) m_keyState, (void *) keyState, numKeys * sizeof(bool));
    m_numKeys = numKeys;

    // read the mouse
    m_mouseButtons = SDL_GetRelativeMouseState(&m_relativeMouse.x, &m_relativeMouse.y);
}

/// Update a frame of gameplay
void Game::UpdateGame()
{
    PROFILE_SCOPE(GameUpdate);
    // calculate the length of this frame
    float deltaTime = 0.0f;

    // Compute delta time
    uint64_t tickNow = SDL_GetTicks();

    Profiler::Timer *frameCap_timer = Profiler::Get()->GetTimer("FrameLock16ms");
    frameCap_timer->Start();
    // wait until 16 ms have elapsed at least
    while (tickNow - m_ticksCount < 16)
    {
        tickNow = SDL_GetTicks();
    }
    frameCap_timer->Stop();

    // Get deltaTime in seconds
    deltaTime = (tickNow - m_ticksCount) / 1000.0f;
    m_ticksCount = tickNow;
    // cap it to 33 ms
    if (deltaTime > 0.033f)
    {
        deltaTime = 0.033f;
    }

    Profiler::Timer *actor_timer = Profiler::Get()->GetTimer("ActorUpdate");
    actor_timer->Start();
    for (auto actor: m_actors)
    {
        actor->Update(deltaTime);
    }
    actor_timer->Stop();

    m_qbertLevel->Update(deltaTime);

    Profiler::Timer *jobWait_timer = Profiler::Get()->GetTimer("JobManager::WaitForJobs");
    jobWait_timer->Start();
    JobManager::Get()->WaitForJobs();
    jobWait_timer->Stop();
}

void Game::RenderFrame()
{
    PROFILE_SCOPE(GameRenderFrame);
    // acquire the command buffer
    SDL_GPUCommandBuffer *commandBuffer = m_renderer.BeginCommandBuffer();
    if (nullptr == commandBuffer)
        return;

    m_lighting->SetActive(m_camera, commandBuffer);

    // ===== PASS 1: Render scene to FULLSIZED buffer =====
    SDL_GPURenderPass *renderPass = m_renderer.ClearScreen(commandBuffer, SDL_FColor{0.0f, 0.2f, 0.4f, 1.0f}, true);
    m_camera->SetActive(commandBuffer);
    for (Actor *actor: m_actors)
    {
        actor->Draw(commandBuffer, renderPass);
    }
    m_renderer.EndRenderPass(renderPass);

    // ===== PASS 2: Apply bloom mask to create HALF buffer =====
    SDL_GPURenderPass *bloomPass = m_renderer.BeginRenderPass(commandBuffer, m_offscreenTargets[OffscreenTarget::HALF],
                                                              SDL_FColor(0.0f, 0.0f, 0.0f, 1.0f));
    m_offscreenTargets[OffscreenTarget::FULLSIZED]->SetActive(bloomPass, 0);
    m_bloomMaskShader->SetActive(bloomPass);
    m_fullscreenBuffer->Draw(commandBuffer, bloomPass);
    m_renderer.EndRenderPass(bloomPass);

    // ===== PASS 3: Downsample HALF to QUARTER buffer =====
    SDL_GPURenderPass *downsamplePass = m_renderer.BeginRenderPass(commandBuffer,
                                                                   m_offscreenTargets[OffscreenTarget::QUARTER],
                                                                   SDL_FColor(0.0f, 0.0f, 0.0f, 1.0f));
    m_offscreenTargets[OffscreenTarget::HALF]->SetActive(downsamplePass, 0);
    m_copyShader->SetActive(downsamplePass);
    m_fullscreenBuffer->Draw(commandBuffer, downsamplePass);
    m_renderer.EndRenderPass(downsamplePass);

    // ===== PASS 4: Vertical Blur Pass (QUARTER → QUARTER2) =====
    SDL_GPURenderPass *blurPass1 = m_renderer.BeginRenderPass(commandBuffer,
                                                              m_offscreenTargets[OffscreenTarget::QUARTER2],
                                                              SDL_FColor(0.0f, 0.0f, 0.0f, 1.0f));
    m_offscreenTargets[OffscreenTarget::QUARTER]->SetActive(blurPass1, 0);
    m_verticalGaussianShader->SetActive(blurPass1);
    m_fullscreenBuffer->Draw(commandBuffer, blurPass1);
    m_renderer.EndRenderPass(blurPass1);

    // ===== PASS 5: Horizontal Blur Pass (QUARTER2 → QUARTER) =====
    SDL_GPURenderPass *blurPass2 = m_renderer.BeginRenderPass(commandBuffer,
                                                              m_offscreenTargets[OffscreenTarget::QUARTER],
                                                              SDL_FColor(0.0f, 0.0f, 0.0f, 1.0f));
    m_offscreenTargets[OffscreenTarget::QUARTER2]->SetActive(blurPass2, 0);
    m_horizontalGaussianShader->SetActive(blurPass2);
    m_fullscreenBuffer->Draw(commandBuffer, blurPass2);
    m_renderer.EndRenderPass(blurPass2);

    // ===== PASS 6: Copy original scene (FULLSIZED) to back buffer =====
    SDL_GPURenderPass *compositePass = m_renderer.BeginRenderPass(commandBuffer, SDL_FColor(0.0f, 0.0f, 0.0f, 1.0f),
                                                                  false);
    m_offscreenTargets[OffscreenTarget::FULLSIZED]->SetActive(compositePass, 0);
    m_copy2Shader->SetActive(compositePass);
    m_fullscreenBuffer->Draw(commandBuffer, compositePass);
    m_renderer.EndRenderPass(compositePass);

    // ===== PASS 7: Additively blend bloom (QUARTER) on top of back buffer =====
    SDL_GPURenderPass *bloomCompositePass = m_renderer.BeginRenderPass(commandBuffer,
                                                                       SDL_FColor(0.0f, 0.0f, 0.0f, 1.0f), false);
    m_offscreenTargets[OffscreenTarget::QUARTER]->SetActive(bloomCompositePass, 0);
    m_copy3Shader->SetActive(bloomCompositePass);
    m_fullscreenBuffer->Draw(commandBuffer, bloomCompositePass);
    m_renderer.EndRenderPass(bloomCompositePass);

    // submit the command buffer
    m_renderer.EndCommandBuffer(commandBuffer);
}

/// Check the current status of a single key on the keyboard
/// @param key the key to check as a SDL_Scancode (SDL_SCANCODE_UP)
/// @return true if the key is currently held down or false if it is not
bool Game::IsKeyHeld(int key) const
{
    if (key >= m_numKeys)
        return false;
    return m_keyState[key];
}

bool Game::IsKeyHit(const int key) const
{
    return IsKeyHeld(key) && !m_lastKeyState[key];
}

/// Returns the mouse movement since the previous frame
/// @return movement in normalized device coordinates [-1.0, 1.0]
Vector2 Game::GetRelativeMouse() const
{
    Vector2 move = m_relativeMouse;
    return move * Vector2(1.0f / m_renderer.GetScreenWidth(), 1.0f / m_renderer.GetScreenHeight());
}

/// Returns the current state of the mouse buttons.
/// Use SDL_BUTTON_MASK() to ask for the button(s) you are interested in.
/// @return a SDL_MouseButtonFlags
uint32_t Game::GetMouseButtons() const
{
    return m_mouseButtons;
}

/// Find an actor in m_actors and remove + delete it if found.
void Game::RemoveActor(const Actor *actor)
{
    for (auto it = m_actors.begin(); it != m_actors.end(); ++it)
    {
        if (*it == actor)
        {
            m_actors.erase(it);
            delete actor;
            return;
        }
    }
}

/// Load a level creating instances of all the actors listed
/// @param fileName the name of the level to load ("Assets/Levels/Level06.itplevel")
/// @return true on success or false on failure
bool Game::LoadLevel(const char *fileName)
{
    PROFILE_SCOPE(LoadLevel);

    std::ifstream file(fileName);
    if (!file.is_open())
    {
        return false;
    }

    std::stringstream fileStream;
    fileStream << file.rdbuf();
    std::string contents = fileStream.str();
    rapidjson::StringStream jsonStr(contents.c_str());
    rapidjson::Document doc;
    doc.ParseStream(jsonStr);

    if (!doc.IsObject())
    {
        return false;
    }

    std::string str = doc["metadata"]["type"].GetString();
    int ver = doc["metadata"]["version"].GetInt();

    // Check the metadata
    if (!doc["metadata"].IsObject() ||
        str != "itplevel" ||
        ver != 3)
    {
        return false;
    }

    // Load camera data
    auto &camera = doc["camera"];
    if (camera.IsObject())
    {
        Vector3 cameraPos;
        Quaternion cameraRot;
        if (GetVectorFromJSON(camera, "position", cameraPos) &&
            GetQuaternionFromJSON(camera, "rotation", cameraRot))
        {
            // Build the world-to-camera (view) matrix from position and rotation
            // Camera-to-world = rotate * translate, so world-to-camera = inverse
            Matrix4 cameraToWorld = Matrix4::CreateFromQuaternion(cameraRot) * Matrix4::CreateTranslation(cameraPos);
            cameraToWorld.Invert();
            m_camera->SetViewMatrix(cameraToWorld);
        }
    }

    // Load lighting data
    auto &lighting = doc["lightingData"];
    if (lighting.IsObject())
    {
        Vector3 ambient;
        if (GetVectorFromJSON(lighting, "ambient", ambient))
        {
            m_lighting->SetAmbientLight(ambient);
        }
    }

    // Generate Qbert Level
    Vector3 primary, secondary, activated;
    auto &qbertColors = doc["qbertColors"];
    if (qbertColors.IsObject())
    {
        GetVectorFromJSON(qbertColors, "primary", primary);
        GetVectorFromJSON(qbertColors, "secondary", secondary);
        GetVectorFromJSON(qbertColors, "activated", activated);
    }

    //Load actors from level
    auto &objs = doc["actors"];
    if (objs.IsArray())
    {
        for (rapidjson::SizeType i = 0; i < objs.Size(); i++)
        {
            if (!objs[i].IsObject()) continue;

            Actor *actor;
            Vector3 position = Vector3::Zero;
            Quaternion rotation = Quaternion::Identity;
            std::string meshName;
            float scale = 1.0f;
            GetQuaternionFromJSON(objs[i], "rotation", rotation);
            GetFloatFromJSON(objs[i], "scale", scale);
            GetVectorFromJSON(objs[i], "position", position);

            GetStringFromJSON(objs[i], "mesh", meshName);
            if (Mesh *mesh = m_assetManager.LoadMesh(meshName); mesh != nullptr && mesh->IsSkinned())
            {
                actor = new SkinnedObj(&m_renderer, mesh);
            } else
            {
                actor = new Actor(&m_renderer, mesh);
            }
            actor->SetPosition(position);
            actor->m_constants.modelToWorld = Matrix4::CreateScale(scale)
                                              * Matrix4::CreateFromQuaternion(rotation)
                                              * Matrix4::CreateTranslation(position);
            m_actors.push_back(actor);

            //Load Components
            auto &components = objs[i]["components"];
            for (rapidjson::SizeType j = 0; j < components.Size(); j++)
            {
                if (components[j].IsObject())
                {
                    std::string type;
                    GetStringFromJSON(components[j], "type", type);
                    if (type == "PointLight")
                    {
                        PointLight *light = new PointLight(actor, m_lighting);
                        light->LoadProperties(components[j]);
                        actor->AddComponent(light);
                    } else if (type == "Character")
                    {
                        Character *character = new Character(dynamic_cast<SkinnedObj *>(actor));
                        character->LoadProperties(components[j]);
                        actor->AddComponent(character);
                    } else if (type == "Player")
                    {
                        Player *player = new Player(dynamic_cast<SkinnedObj *>(actor), this);
                        player->LoadProperties(components[j]);
                        actor->AddComponent(player);
                        m_player = player;
                    } else if (type == "FollowCam")
                    {
                        FollowCam *cam = new FollowCam(actor, this);
                        cam->LoadProperties(components[j]);
                        actor->AddComponent(cam);
                    } else if (type == "CollisionBox")
                    {
                        auto box = new CollisionBox(actor);
                        box->LoadProperties(components[j]);
                        actor->AddComponent(box);
                        m_physics->AddObj(box);
                    } else if (type == "SimpleRotate")
                    {
                        auto *rot = new SimpleRotate(actor);
                        rot->LoadProperties(components[j]);
                        actor->AddComponent(rot);
                    }
                }
            }
        }
    }
    m_qbertLevel = new QbertLevel(primary, secondary, activated, m_player);
    m_player->SetCurrentCube(m_qbertLevel->GetQbertCube(0, 0));
    return true;
}

/// Load all the shaders we will need
/// We load all shaders up front (Loading shaders takes a while and should not be done during the frame)
/// Ideally, we would pre-compile the shaders and load the binaries, but we'll be loading them as raw HLSL
void Game::LoadShaders()
{
    PROFILE_SCOPE(LoadShaders);
    constexpr SDL_GPUVertexAttribute meshAttributes[] = {
        {0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(VertexData, pos)},
        {1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(VertexData, normal)},
        {2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(VertexData, uv)}
    };

    constexpr SDL_GPUVertexAttribute uvAttributes[] = {
        {0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(VertexUV, pos)},
        {1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(VertexUV, uv)}
    };

    constexpr SDL_GPUVertexAttribute skinnedMeshAttributes[] = {
        {0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(VertexDataSkinned, pos)},
        {1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, offsetof(VertexDataSkinned, normal)},
        {2, 0, SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4, offsetof(VertexDataSkinned, bones)},
        {3, 0, SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM, offsetof(VertexDataSkinned, weights)},
        {4, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, offsetof(VertexDataSkinned, uv)}
    };

    m_assetManager.SetShader("Phong", new Shader(Renderer::Get(), "Shaders/Phong.hlsl"));
    m_assetManager.GetShader("Phong")->SetColorTarget(m_offscreenTargets[FULLSIZED]);
    m_assetManager.GetShader("Phong")->CreatePipeline(meshAttributes, ARRAY_SIZE(meshAttributes),
                                                      sizeof(VertexData));

    m_assetManager.SetShader("PhongQbertCube", new Shader(Renderer::Get(), "Shaders/PhongQbertCube.hlsl"));
    m_assetManager.GetShader("PhongQbertCube")->SetColorTarget(m_offscreenTargets[FULLSIZED]);
    m_assetManager.GetShader("PhongQbertCube")->CreatePipeline(meshAttributes, ARRAY_SIZE(meshAttributes),
                                                               sizeof(VertexData));

    m_assetManager.SetShader("Unlit", new Shader(Renderer::Get(), "Shaders/Unlit.hlsl"));
    m_assetManager.GetShader("Unlit")->SetColorTarget(m_offscreenTargets[FULLSIZED]);
    m_assetManager.GetShader("Unlit")->CreatePipeline(meshAttributes, ARRAY_SIZE(meshAttributes),
                                                      sizeof(VertexData));

    m_assetManager.SetShader("Skinned", new Shader(Renderer::Get(), "Shaders/Skinned.hlsl"));
    m_assetManager.GetShader("Skinned")->SetColorTarget(m_offscreenTargets[FULLSIZED]);
    m_assetManager.GetShader("Skinned")->CreatePipeline(skinnedMeshAttributes, ARRAY_SIZE(skinnedMeshAttributes),
                                                        sizeof(VertexDataSkinned));

    m_assetManager.SetShader("Copy", new Shader(Renderer::Get(), "Shaders/Copy.hlsl"));
    m_copyShader = m_assetManager.GetShader("Copy");
    m_copyShader->SetColorTarget(m_offscreenTargets[OffscreenTarget::QUARTER]); // Copy targets QUARTER
    m_copyShader->SetDepthFlags(DepthFlags::NONE);
    m_copyShader->CreatePipeline(uvAttributes, ARRAY_SIZE(uvAttributes), sizeof(VertexUV));

    m_assetManager.SetShader("Copy2", new Shader(Renderer::Get(), "Shaders/Copy2.hlsl"));
    m_copy2Shader = m_assetManager.GetShader("Copy2");
    m_copy2Shader->SetColorTarget(nullptr); // Copy2 targets back buffer
    m_copy2Shader->SetDepthFlags(DepthFlags::NONE);
    m_copy2Shader->CreatePipeline(uvAttributes, ARRAY_SIZE(uvAttributes), sizeof(VertexUV));

    m_assetManager.SetShader("Copy3", new Shader(Renderer::Get(), "Shaders/Copy3.hlsl"));
    m_copy3Shader = m_assetManager.GetShader("Copy3");
    m_copy3Shader->SetColorTarget(nullptr); // Copy3 targets back buffer
    m_copy3Shader->SetDepthFlags(DepthFlags::NONE);
    // Enable additive blending for bloom composition
    m_copy3Shader->SetBlendEnabled(true);
    m_copy3Shader->SetBlendFactors(SDL_GPU_BLENDFACTOR_ONE, SDL_GPU_BLENDFACTOR_ONE);
    m_copy3Shader->CreatePipeline(uvAttributes, ARRAY_SIZE(uvAttributes), sizeof(VertexUV));

    m_assetManager.SetShader("BloomMask", new Shader(Renderer::Get(), "Shaders/BloomMask.hlsl"));
    m_bloomMaskShader = m_assetManager.GetShader("BloomMask");
    m_bloomMaskShader->SetColorTarget(m_offscreenTargets[OffscreenTarget::HALF]); // BloomMask targets HALF
    m_bloomMaskShader->SetDepthFlags(DepthFlags::NONE);
    m_bloomMaskShader->CreatePipeline(uvAttributes, ARRAY_SIZE(uvAttributes), sizeof(VertexUV));

    m_assetManager.SetShader("VerticalGaussianBlur", new Shader(Renderer::Get(), "Shaders/VerticalGaussianBlur.hlsl"));
    m_verticalGaussianShader = m_assetManager.GetShader("VerticalGaussianBlur");
    m_verticalGaussianShader->SetColorTarget(m_offscreenTargets[OffscreenTarget::QUARTER2]);
    m_verticalGaussianShader->SetDepthFlags(DepthFlags::NONE);
    m_verticalGaussianShader->CreatePipeline(uvAttributes, ARRAY_SIZE(uvAttributes), sizeof(VertexUV));

    m_assetManager.SetShader("HorizontalGaussianBlur",
                             new Shader(Renderer::Get(), "Shaders/HorizontalGaussianBlur.hlsl"));
    m_horizontalGaussianShader = m_assetManager.GetShader("HorizontalGaussianBlur");
    m_horizontalGaussianShader->SetColorTarget(m_offscreenTargets[OffscreenTarget::QUARTER]);
    m_horizontalGaussianShader->SetDepthFlags(DepthFlags::NONE);
    m_horizontalGaussianShader->CreatePipeline(uvAttributes, ARRAY_SIZE(uvAttributes), sizeof(VertexUV));
}
