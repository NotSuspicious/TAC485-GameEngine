//
// Created by William Zhao on 2/25/26.
//

#include "Lighting.h"
#include "Profiler.h"

Lighting::PointLightData *Lighting::AllocateLight()
{
    for (int i = 0; i < POINT_LIGHT_COUNT; i++)
    {
        if (!m_constants.pointLights[i].isEnabled)
        {
            m_constants.pointLights[i].isEnabled = true;
            return &m_constants.pointLights[i];
        }
    }
    return nullptr;
}

void Lighting::FreeLight(Lighting::PointLightData *pLight)
{
    pLight->isEnabled = false;
}

void Lighting::SetAmbientLight(const Vector3 &color)
{
    m_constants.ambientColor = color;
}

const Vector3 &Lighting::GetAmbientLight() const
{
    return m_constants.ambientColor;
}

void Lighting::SetActive(Camera *camera, SDL_GPUCommandBuffer *commandBuffer)
{
    PROFILE_SCOPE(LightingSetActive);
    Matrix4 viewToWorld = camera->GetConstants().view_projection;
    viewToWorld.Invert();
    Vector3 cameraPos = viewToWorld.GetTranslation();
    m_constants.cameraPosition = cameraPos;
    SDL_PushGPUFragmentUniformData(commandBuffer, Renderer::ConstantBuffer_Fragment::CONSTANT_FRAGMENT_LIGHTS,
                                   &m_constants,
                                   sizeof(LightingConstants));
}
