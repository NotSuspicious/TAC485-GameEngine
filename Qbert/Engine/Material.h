//
// Created by William Zhao on 2/21/26.
//

#ifndef GAME_MATERIAL_H
#define GAME_MATERIAL_H

#include "Renderer.h"
#include "Texture.h"
#include "EngineMath.h"
#include "Shader.h"

class Material
{
public:
    struct Data
    {
        Vector3 diffuse = Vector3(1.0f, 1.0f, 1.0f);
        float padding0 = 0.0f;
        Vector3 specular = Vector3(1.0f, 1.0f, 1.0f);
        float specularPower = 1.0f;
    };

    Material();

    ~Material();

    void SetActive(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass);

    void SetShader(Shader *shader);

    void SetTexture(int slot, const Texture *texture);

    void SetDiffuseColor(const Vector3 &color);

    void SetSpecularColor(const Vector3 &color);

    void SetSpecularPower(float power);

    bool Load(const char *fileName, AssetManager *pAssetManager);

    static Material *StaticLoad(const char *fileName, AssetManager *pManager);

private:
    Shader *m_shader = nullptr;

    const Texture *m_textures[Renderer::TEXTURE_SLOT_TOTAL] = {};
    Data m_data;
};


#endif //GAME_MATERIAL_H
