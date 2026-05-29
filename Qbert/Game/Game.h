#pragma once

#include "Renderer.h"
#include "EngineMath.h"
#include "AssetManager.h"

class Game
{
public:
    static Game *Get();

    ~Game();

    bool Init(int width, int height);

    void Shutdown();

    void ProcessInput();

    void UpdateGame();

    void RenderFrame();

    // Polling Input Status
    [[nodiscard]]

    bool IsKeyHeld(int key) const;

    [[nodiscard]] bool IsKeyHit(int key) const;

    Vector2 GetRelativeMouse() const;

    uint32_t GetMouseButtons() const;

    AssetManager &GetAssetManager() { return m_assetManager; }

    float rotationAngle = 0.0f;

    class Camera *GetCamera() const { return m_camera; }

    class Physics *GetPhysics() const { return m_physics; }

    class QbertLevel *GetQbertLevel() const { return m_qbertLevel; }

    std::vector<class Actor *> &GetActors() { return m_actors; }

    void RemoveActor(const class Actor *actor);

    enum OffscreenTarget
    {
        FULLSIZED,
        HALF,
        QUARTER,
        QUARTER2
    };

    static constexpr int OFFSCREEN_TARGET_COUNT = 4;

    class Texture *GetOffscreenTarget(const OffscreenTarget target) const
    {
        return m_offscreenTargets[static_cast<int>(target)];
    }

private:
    Game();

    static Game *s_instance;

    Renderer m_renderer;
    uint64_t m_ticksCount = 0;

    // Input
    bool *m_keyState = nullptr;
    bool *m_lastKeyState = nullptr;
    int m_numKeys = 0;
    uint32_t m_mouseButtons = 0;
    uint32_t m_lastMouseButtons = 0;
    Vector2 m_relativeMouse = Vector2::Zero;

    class Camera *m_camera = nullptr;

    class Lighting *m_lighting = nullptr;

    class Physics *m_physics = nullptr;

    class Player *m_player = nullptr;

    class Texture *m_offscreenTargets[OFFSCREEN_TARGET_COUNT];

    class VertexBuffer *m_fullscreenBuffer = nullptr;

    class Shader *m_bloomMaskShader = nullptr;
    class Shader *m_copyShader = nullptr;
    class Shader *m_copy2Shader = nullptr;
    class Shader *m_copy3Shader = nullptr;
    class Shader *m_verticalGaussianShader = nullptr;
    class Shader *m_horizontalGaussianShader = nullptr;

    AssetManager m_assetManager;

    std::vector<class Actor *> m_actors;

    class QbertLevel *m_qbertLevel;

    void LoadShaders();

    bool LoadLevel(const char *fileName);
};
