#pragma once

namespace dae
{
    // Forward declarations
    class Effect;
    class Texture;

    struct Vertex
    {
        Vector3  position = { 0.0f, 0.0f, 0.0f };
        ColorRGB color = colors::White;
        Vector2  uv = { 0.0f, 1.0f };
        Vector3  normal = { 0.0f, 0.0f, 1.0f };
        Vector3  tangent = { 0.0f, 0.0f, 1.0f };
    };

    class Mesh
    {
    public:
        Mesh(ID3D11Device* devicePtr, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        ~Mesh();

        Mesh(const Mesh& other) = delete;
        Mesh(Mesh&& other) noexcept = delete;
        Mesh& operator=(const Mesh& other) = delete;
        Mesh& operator=(Mesh&& other) noexcept = delete;

        void Render() const;
        void UpdateMatrix(const Matrix& viewMatrix, const Matrix& projectionMatrix) const;

        void SetDiffuseMap(const Texture* diffuseTexturePtr) const;
        void SetNormalMap(const Texture* normalMapTexturePtr) const;
        void SetSpecularMap(const Texture* specularTexturePtr) const;
        void SetGlossinessMap(const Texture* glossinessTexturePtr) const;

        void SetPassIdx(UINT passIdx) { m_PassIdx = passIdx; }
        void SetDeltaTime(float dt) const { m_TimePtr->SetFloat(dt); };
        void SetCameraPosition(const Vector3& viewDirection) const { m_CameraPosPtr->SetFloatVector(reinterpret_cast<const float*>(&viewDirection)); };
        void SetUseNormalMap(bool useNormalMap) const { m_UseNormalMapPtr->SetBool(useNormalMap); };
    private:
        ID3D11Device* m_DevicePtr = nullptr;
        ID3D11DeviceContext* m_DeviceContextPtr = nullptr;

        ID3D11Buffer* m_VertexBufferPtr = nullptr;
        ID3D11InputLayout* m_InputLayoutPtr = nullptr;
        ID3D11Buffer* m_IndexBufferPtr = nullptr;

        Effect* m_EffectPtr = nullptr;
        ID3DX11EffectTechnique* m_TechniquePtr = nullptr;
        ID3DX11EffectMatrixVariable* m_WorldViewProjectionMatrixPtr = nullptr;

        ID3DX11EffectShaderResourceVariable* m_DiffuseMapPtr = nullptr;
        ID3DX11EffectShaderResourceVariable* m_NormalMapPtr = nullptr;
        ID3DX11EffectShaderResourceVariable* m_SpecularMapPtr = nullptr;
        ID3DX11EffectShaderResourceVariable* m_GlossinessMapPtr = nullptr;

        ID3DX11EffectScalarVariable* m_TimePtr = nullptr;
        ID3DX11EffectVectorVariable* m_CameraPosPtr = nullptr;
        ID3DX11EffectScalarVariable* m_UseNormalMapPtr = nullptr;

        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        uint32_t m_NumIndices = 0;

        UINT m_PassIdx = 0;
    };
}