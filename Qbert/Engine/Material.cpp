//
// Created by William Zhao on 2/21/26.
//

#include "Material.h"
#include "AssetManager.h"
#include "JsonUtil.h"
#include "Profiler.h"

Material::~Material()
{
}

Material::Material()
{
}

void Material::SetSpecularPower(float power)
{
    m_data.specularPower = power;
}

void Material::SetSpecularColor(const Vector3 &color)
{
    m_data.specular = color;
}

void Material::SetDiffuseColor(const Vector3 &color)
{
    m_data.diffuse = color;
}

void Material::SetTexture(int slot, const Texture *texture)
{
    m_textures[slot] = texture;
}

void Material::SetShader(Shader *shader)
{
    m_shader = shader;
}

void Material::SetActive(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass)
{
    PROFILE_SCOPE(MaterialSetActive);
    m_shader->SetActive(renderPass);
    for (int i = 0; i < Renderer::TEXTURE_SLOT_TOTAL; i++)
    {
        if (m_textures[i])
        {
            m_textures[i]->SetActive(renderPass, i);
        }
    }
    SDL_PushGPUFragmentUniformData(commandBuffer, Renderer::ConstantBuffer_Fragment::CONSTANT_FRAGMENT_MATERIAL,
                                   &m_data, sizeof(Data));
}

bool Material::Load(const char *fileName, AssetManager *pAssetManager)
{
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
        str != "itpmat" ||
        ver != 1)
    {
        return false;
    }
    // Load Shader
    std::string shaderName;
    if (false == GetStringFromJSON(doc, "shader", shaderName))
        return false;
    m_shader = pAssetManager->GetShader(shaderName);
    DbgAssert(nullptr != m_shader, "Material unable to load shader"); {
        // Load textures
        const rapidjson::Value &textures = doc["textures"];
        if (textures.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < textures.Size(); i++)
            {
                if (i < Renderer::TEXTURE_SLOT_TOTAL)
                {
                    m_textures[i] = pAssetManager->LoadTexture(textures[i].GetString());
                }
            }
        }
    }
    // Load the Lighting Parameters
    GetVectorFromJSON(doc, "diffuseColor", m_data.diffuse);
    GetVectorFromJSON(doc, "specularColor", m_data.specular);
    GetFloatFromJSON(doc, "specularPower", m_data.specularPower);
    return true;
}

Material *Material::StaticLoad(const char *fileName, AssetManager *pManager)
{
    Material * pMaterial = new Material();
    if (!pMaterial->Load(fileName, pManager))
    {
        delete pMaterial;
        return new Material();
    }
    return pMaterial;
}
