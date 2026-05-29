//
// Created by William Zhao on 4/30/26.
//

#ifndef GAME_QBERTCUBE_H
#define GAME_QBERTCUBE_H
#include "Actor.h"


class QbertCube : public Actor
{
public:
    struct Constants
    {
        Vector3 primaryColor = Vector3(0.0f, 0.0f, 0.8f);
        float padding0;
        Vector3 secondaryColor = Vector3(0.8f, 0.8f, 0.8f);
        float padding1;
        Vector3 activatedColor = Vector3(0.0f, 0.8f, 0.8f);
        bool isActivated = false;
    };

    void SetColors(Vector3 primary, Vector3 secondary, Vector3 activated);

    QbertCube(class Renderer *renderer);

    ~QbertCube() override;

    void Draw(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) override;

    void Update(float deltaTime) override;

    void SetActive(SDL_GPUCommandBuffer *command_buffer) const;

    bool IsActivated() const { return m_constants.isActivated; }

    void SetActivation(const bool b) { m_constants.isActivated = b; }

    Constants m_constants;
};


#endif //GAME_QBERTCUBE_H
