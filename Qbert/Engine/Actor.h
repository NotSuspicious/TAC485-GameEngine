//
// Created by William Zhao on 2/2/26.
//

#ifndef ACTOR_H
#define ACTOR_H

#include "EngineMath.h"


class Actor
{
public:
    Actor(class Renderer *renderer, const class Mesh *mesh);

    virtual ~Actor();

    virtual void Draw(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass);

    struct Constants
    {
        Matrix4 modelToWorld = Matrix4::Identity;
    };

    Constants m_constants;

    void AddComponent(class Component *pComp);

    const std::vector<class Component *> &GetComponents() const { return m_components; }

    virtual void Update(float deltaTime);

    [[nodiscard]] const Vector3 &GetPosition() const;

    void SetPosition(const Vector3 &mPosition);

    // Scale getter/setter (uniform scale)
    float GetScale() const;

    void SetScale(float scale);

    // Rotation getter/setter
    const Quaternion &GetRotation() const;

    void SetRotation(const Quaternion &rotation);

    void UpdateModelToWorld();

protected:
    const Mesh *m_mesh = nullptr;

    class Renderer *m_renderer = nullptr;

    std::vector<class Component *> m_components;

    Vector3 m_position;

    // Uniform scale for the actor (default 1.0)
    float m_scale = 1.0f;

    // Orientation for the actor (default identity)
    Quaternion m_rotation = Quaternion::Identity;
};


#endif //ACTOR_H
