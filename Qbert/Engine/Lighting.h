//
// Created by William Zhao on 2/25/26.
//

#ifndef GAME_LIGHTING_H
#define GAME_LIGHTING_H


#include "EngineMath.h"
#include "Camera.h"

class Lighting
{
public:
    struct PointLightData
    {
        Vector3 lightColor;
        float padding0;
        Vector3 position;
        float padding1;
        float innerRadius;
        float outerRadius;
        bool isEnabled = false;
        float padding2;
    };

    const static int POINT_LIGHT_COUNT = 8;

    struct LightingConstants
    {
        Vector3 cameraPosition;
        float padding0;
        Vector3 ambientColor;
        float padding1;
        PointLightData pointLights[POINT_LIGHT_COUNT];
    };

    PointLightData *AllocateLight();

    void FreeLight(PointLightData *pLight);

    void SetAmbientLight(const Vector3 &color);

    [[nodiscard]] const Vector3 &GetAmbientLight() const;

    void SetActive(Camera *camera, SDL_GPUCommandBuffer *commandBuffer);

private:
    LightingConstants m_constants;
};


#endif //GAME_LIGHTING_H
