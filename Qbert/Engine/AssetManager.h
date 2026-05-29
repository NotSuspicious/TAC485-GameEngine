#pragma once

#include "AssetCache.h"
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "Animation.h"

class AssetManager
{
public:
    AssetManager();

    ~AssetManager();

    void Clear();

    Shader *GetShader(const std::string &shaderName);

    void SetShader(const std::string &shaderName, Shader *pShader);

    Texture *LoadTexture(const std::string &fileName);

    Material *LoadMaterial(const std::string &materialName);

    Mesh *LoadMesh(const std::string &fileName);

    Skeleton *LoadSkeleton(const std::string &fileName);

    Animation *LoadAnimation(const std::string &fileName);

private:
    AssetCache<Shader> m_shaderCache;
    AssetCache<Texture> m_textureCache;
    AssetCache<Material> m_materialCache;
    AssetCache<Mesh> m_meshCache;
    AssetCache<Skeleton> m_skeletonCache;
    AssetCache<Animation> m_animationCache;
};
