//
// Created by William Zhao on 2/4/26.
//

#include "Camera.h"
#include "Profiler.h"

Camera::Camera(Renderer *renderer)
    : m_renderer(renderer)
{
    m_worldToCameraMatrix = Matrix4::CreateTranslation(Vector3(500.0f, 0.0f, 0.0f));
    m_projectionMatrix = Matrix4::CreateRotationY(-Math::PiOver2)
                         * Matrix4::CreateRotationZ(-Math::PiOver2)
                         * Matrix4::CreatePerspectiveFOV(Math::ToRadians(70.0f),
                                                         m_renderer->GetScreenWidth(), m_renderer->GetScreenHeight(),
                                                         25.0f, 10000.0f);
    // m_projectionMatrix = Matrix4::CreatePerspective(fov, aspect, nearPlane, farPlane);
}

Camera::~Camera()
{
}

void Camera::SetViewMatrix(const Matrix4 &viewMatrix)
{
    m_worldToCameraMatrix = viewMatrix;
}

void Camera::SetPosition(const Vector3 &position)
{
    m_worldToCameraMatrix = Matrix4::CreateTranslation(position);
}

void Camera::SetRotation(const Quaternion &rotation)
{
    Matrix4 rotationMatrix = Matrix4::CreateFromQuaternion(rotation);
    m_worldToCameraMatrix = rotationMatrix * m_worldToCameraMatrix;
}

void Camera::SetActive(SDL_GPUCommandBuffer *command_buffer)
{
    PROFILE_SCOPE(CameraSetActive);
    m_constants.view_projection = m_worldToCameraMatrix * m_projectionMatrix;
    SDL_PushGPUVertexUniformData(command_buffer, Renderer::ConstantBuffer_Vertex::CONSTANT_VERTEX_CAMERA, &m_constants,
                                 sizeof(Camera::Constants));
}
