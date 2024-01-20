#pragma once
namespace dae {
    class Matrix;


    class Camera
    {
    public:
        Camera() = default;
        Camera(const Vector3& _origin, float _fovAngle);

        Camera(const Camera& other) = delete;
        Camera(Camera&& other) noexcept = delete;
        Camera& operator=(const Camera& other) = delete;
        Camera& operator=(Camera&& other) noexcept = delete;

        void Initialize(float _fovAngle = 90.f, Vector3 _origin = { 0.0f, 0.0f, 0.0f }, float nearPlane = 1.0f, float farPlane = 1000.0f);
        void Update(const Timer* pTimer);

        void CalculateViewMatrix();
        void CalculateProjectionMatrix();

        void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }
        Vector3 GetPosition() const { return m_Origin; }

        Matrix GetInverseViewMatrix() const { return m_InverseViewMatrix; }
        Matrix GetProjectionMatrix()  const { return m_ProjectionMatrix; }

    private:
        void CalculateFOV() { m_FOV = tanf((m_FOVAngle * TO_RADIANS) * 0.5f); };

        Matrix m_InverseViewMatrix{};
        Matrix m_ViewMatrix{};
        Matrix m_ProjectionMatrix{};

        Vector3 m_Origin{};
        float   m_FOVAngle{};
        float   m_FOV{};
        float   m_AspectRatio{};
        float   m_NearPlane{};
        float   m_FarPlane{};

        Vector3 m_Right{ Vector3::UnitX };
        Vector3 m_Up{ Vector3::UnitY };
        Vector3 m_Forward{ Vector3::UnitZ };

        float  m_TotalPitch{};
        float  m_TotalYaw{};
    };

}
