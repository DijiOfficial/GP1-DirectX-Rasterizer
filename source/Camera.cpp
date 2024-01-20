#include "pch.h"
#include "Camera.h"

using namespace dae;

Camera::Camera(const Vector3& _origin, float _fovAngle)
    : m_Origin{ _origin }
    , m_FOVAngle{ _fovAngle }
{
}

void Camera::Initialize(float _fovAngle, Vector3 _origin, float nearPlane, float farPLane)
{
    m_FOVAngle = _fovAngle;
    m_FOV = tanf((m_FOVAngle * TO_RADIANS) * 0.5f);
    
    m_Origin = _origin;
    m_NearPlane = nearPlane;
    m_FarPlane = farPLane;
}

void Camera::Update(const Timer* pTimer)
{
    const float deltaTime = pTimer->GetElapsed();

    const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
    int speed{ 25 };

    if (pKeyboardState[SDL_SCANCODE_LSHIFT])
    {
        speed *= 5;
    }
    if (pKeyboardState[SDL_SCANCODE_W] or pKeyboardState[SDL_SCANCODE_UP])
    {
        m_Origin += m_Forward * deltaTime * speed;
    }
    if (pKeyboardState[SDL_SCANCODE_A] or pKeyboardState[SDL_SCANCODE_LEFT])
    {
        m_Origin -= m_Right * deltaTime * speed;
    }
    if (pKeyboardState[SDL_SCANCODE_S] or pKeyboardState[SDL_SCANCODE_DOWN])
    {
        m_Origin -= m_Forward * deltaTime * speed;
    }
    if (pKeyboardState[SDL_SCANCODE_D] or pKeyboardState[SDL_SCANCODE_RIGHT])
    {
        m_Origin += m_Right * deltaTime * speed;
    }

    //Mouse Input
    int mouseX{}, mouseY{};
    int LMB{};
    const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

    if (SDL_GetMouseState(NULL, NULL) == 1 and mouseY)
    {
        m_Origin -= m_Forward * deltaTime * speed * mouseY;
    }
    if (SDL_GetMouseState(NULL, NULL) == 5 and mouseY)
    {
        m_Origin -= m_Up * deltaTime * speed * mouseY;
    }

    if (SDL_GetMouseState(NULL, NULL) == 1 and mouseX)
    {
        m_TotalYaw += mouseX * speed * deltaTime * speed;
    }

    if (SDL_GetMouseState(NULL, NULL) == 4 and (mouseX or mouseY))
    {
        m_TotalYaw += mouseX * speed * deltaTime * speed;
        m_TotalPitch += mouseY * speed * deltaTime * speed;
    }

    const Matrix rotation{ Matrix::CreateRotationY(m_TotalYaw * TO_RADIANS) * Matrix::CreateRotationX(m_TotalPitch * TO_RADIANS) };
    m_Forward = rotation.TransformVector(Vector3::UnitZ);
   
    CalculateViewMatrix();
    CalculateProjectionMatrix();
}

void Camera::CalculateViewMatrix()
{
    m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();
    m_Up = Vector3::Cross(m_Forward, m_Right).Normalized();

    m_ViewMatrix = {
        Vector4{ m_Right, 0 },
        Vector4{ m_Up, 0 },
        Vector4{ m_Forward, 0 },
        Vector4{ m_Origin, 1 },
    };

    m_InverseViewMatrix = Matrix::Inverse(m_ViewMatrix);
}

void Camera::CalculateProjectionMatrix()
{
    m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
}