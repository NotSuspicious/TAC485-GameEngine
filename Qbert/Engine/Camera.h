//
// Created by William Zhao on 2/4/26.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "EngineMath.h"
#include "Renderer.h"


class Camera
{
public:
    Camera(Renderer *renderer);

    virtual ~Camera();

    struct Constants
    {
        Matrix4 view_projection;
    };

    void SetActive(SDL_GPUCommandBuffer *command_buffer);

    Constants &GetConstants() { return m_constants; }

    void SetViewMatrix(const Matrix4 &viewMatrix);

    Matrix4 GetViewMatrix() { return m_worldToCameraMatrix; }

    void SetPosition(const Vector3 &position);

    void SetRotation(const Quaternion &rotation);

protected:
    Constants m_constants;
    Matrix4 m_worldToCameraMatrix;
    Matrix4 m_projectionMatrix;
    Renderer *m_renderer;
};


#endif //CAMERA_H
