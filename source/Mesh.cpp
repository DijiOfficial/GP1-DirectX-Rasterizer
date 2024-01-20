#include "pch.h"
#include "Mesh.h"
#include "Effect.h"
#include "Texture.h"

#include <cassert>

namespace dae
{
    Mesh::Mesh(ID3D11Device* devicePtr, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
        : m_DevicePtr{ devicePtr }
        , m_Vertices{ vertices }
        , m_Indices{ indices }
    {
        // Shader
        m_EffectPtr = new Effect(m_DevicePtr, L"Resources/PosCol3D.fx");
        m_TechniquePtr = m_EffectPtr->GetTechniqueByName("DefaultTechnique");
        if (!m_TechniquePtr->IsValid())
            assert(false and "Technique not valid!");

        //worldViewProjection
        m_WorldViewProjectionMatrixPtr = m_EffectPtr->GetVariableByName("gWorldViewProj")->AsMatrix();
        if (!m_WorldViewProjectionMatrixPtr->IsValid())
            assert(false and "m_WorldViewProjectionMatrixPtr not valid!");

        //texture
        m_DiffuseMapPtr = m_EffectPtr->GetVariableByName("gDiffuseMap")->AsShaderResource();
        if (!m_DiffuseMapPtr->IsValid())
            assert(false and "Failed to create diffuse map!");

        m_NormalMapPtr = m_EffectPtr->GetVariableByName("gNormalMap")->AsShaderResource();
        if (!m_NormalMapPtr->IsValid())
            assert(false and "Failed to create normal map!");

        m_SpecularMapPtr = m_EffectPtr->GetVariableByName("gSpecularMap")->AsShaderResource();
        if (!m_SpecularMapPtr->IsValid())
            assert(false and "Failed to create specular map!");

        m_GlossinessMapPtr = m_EffectPtr->GetVariableByName("gGlossMap")->AsShaderResource();
        if (!m_GlossinessMapPtr->IsValid())
            assert(false and "Failed to create gloss map!");

        //time, camera, normal map bool
        m_TimePtr = m_EffectPtr->GetVariableByName("gTime")->AsScalar();
        if (!m_TimePtr->IsValid())
            assert(false and "Failed to create time!");

        m_CameraPosPtr = m_EffectPtr->GetVariableByName("gCameraPos")->AsVector();
        if (!m_CameraPosPtr->IsValid())
            assert(false and "Failed to create camera!");

        m_UseNormalMapPtr = m_EffectPtr->GetVariableByName("gUseNormalMap")->AsScalar();
        if (!m_UseNormalMapPtr->IsValid())
            assert(false and "Failed to create normal map bool!");

        m_DevicePtr->GetImmediateContext(&m_DeviceContextPtr);

        //Create Vertex Layout
        static constexpr uint32_t numElements{ 5 };
        D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

        vertexDesc[0].SemanticName = "POSITION";
        vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        vertexDesc[0].AlignedByteOffset = 0;
        vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

        vertexDesc[1].SemanticName = "COLOR";
        vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        vertexDesc[1].AlignedByteOffset = 12;
        vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

        vertexDesc[2].SemanticName = "TEXCOORD";
        vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
        vertexDesc[2].AlignedByteOffset = 24;
        vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

        vertexDesc[3].SemanticName = "NORMAL";
        vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        vertexDesc[3].AlignedByteOffset = 32;
        vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

        vertexDesc[4].SemanticName = "TANGENT";
        vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        vertexDesc[4].AlignedByteOffset = 44;
        vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

        //Create Input Layout
        D3DX11_PASS_DESC passDesc{};
        m_TechniquePtr->GetPassByIndex(0)->GetDesc(&passDesc);

        HRESULT result = m_DevicePtr->CreateInputLayout(
            vertexDesc,
            numElements,
            passDesc.pIAInputSignature,
            passDesc.IAInputSignatureSize,
            &m_InputLayoutPtr);

        if (FAILED(result))
            assert(false and "Failed to create input layout!");

        //Create vertex buffer
        D3D11_BUFFER_DESC bd{};
        bd.Usage = D3D11_USAGE_IMMUTABLE;
        bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(m_Vertices.size());
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = m_Vertices.data();

        result = m_DevicePtr->CreateBuffer(&bd, &initData, &m_VertexBufferPtr);
        if (FAILED(result))
            assert(false and "Failed to create vertex buffer!");

        // Create index buffer
        m_NumIndices = static_cast<uint32_t>(m_Indices.size());
        bd.Usage = D3D11_USAGE_IMMUTABLE;
        bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
        initData.pSysMem = m_Indices.data();
        result = m_DevicePtr->CreateBuffer(&bd, &initData, &m_IndexBufferPtr);
        if (FAILED(result))
            assert(false and "Failed to create index buffer!");
    }

    Mesh::~Mesh()
    {
        if (m_VertexBufferPtr) m_VertexBufferPtr->Release();
        if (m_InputLayoutPtr) m_InputLayoutPtr->Release();
        if (m_IndexBufferPtr) m_IndexBufferPtr->Release();
        if (m_TechniquePtr) m_TechniquePtr->Release();
        if (m_WorldViewProjectionMatrixPtr) m_WorldViewProjectionMatrixPtr->Release();

        if (m_DiffuseMapPtr) m_DiffuseMapPtr->Release();
        if (m_NormalMapPtr) m_NormalMapPtr->Release();
        if (m_SpecularMapPtr) m_SpecularMapPtr->Release();
        if (m_GlossinessMapPtr) m_GlossinessMapPtr->Release();

        if (m_TimePtr) m_TimePtr->Release();
        if (m_CameraPosPtr) m_CameraPosPtr->Release();
        if (m_UseNormalMapPtr) m_UseNormalMapPtr->Release();
        if (m_DeviceContextPtr) m_DeviceContextPtr->Release();

        delete m_EffectPtr;
    }

    void Mesh::Render() const
    {
        //1. Set Primitive Topology
        m_DeviceContextPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        //2. Set Input Layout
        m_DeviceContextPtr->IASetInputLayout(m_InputLayoutPtr);

        //3. Set VertexBuffer
        constexpr UINT stride = sizeof(Vertex);
        constexpr UINT offset = 0;
        m_DeviceContextPtr->IASetVertexBuffers(0, 1, &m_VertexBufferPtr, &stride, &offset);

        //4. Set IndexBuffer
        m_DeviceContextPtr->IASetIndexBuffer(m_IndexBufferPtr, DXGI_FORMAT_R32_UINT, 0);

        //5. Draw
        D3DX11_TECHNIQUE_DESC techDesc;
        m_TechniquePtr->GetDesc(&techDesc);
        if (m_PassIdx < techDesc.Passes)
        {
            m_TechniquePtr->GetPassByIndex(m_PassIdx)->Apply(0, m_DeviceContextPtr);
            m_DeviceContextPtr->DrawIndexed(m_NumIndices, 0, 0);
        }
    }

    void Mesh::UpdateMatrix(const Matrix& viewMatrix, const Matrix& projectionMatrix) const
    {
        const Matrix worldViewProjectionMatrix = viewMatrix * projectionMatrix;
        m_WorldViewProjectionMatrixPtr->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix));
    }

    void Mesh::SetDiffuseMap(const Texture* diffuseTexturePtr) const
    {
        if (diffuseTexturePtr)
            m_DiffuseMapPtr->SetResource(diffuseTexturePtr->GetSRV());
    }

    void Mesh::SetNormalMap(const Texture* normalMapTexturePtr) const
    {
        if (normalMapTexturePtr)
            m_NormalMapPtr->SetResource(normalMapTexturePtr->GetSRV());
    }

    void Mesh::SetSpecularMap(const Texture* specularTexturePtr) const
    {
        if (specularTexturePtr)
            m_SpecularMapPtr->SetResource(specularTexturePtr->GetSRV());
    }

    void Mesh::SetGlossinessMap(const Texture* glossinessTexturePtr) const
    {
        if (glossinessTexturePtr)
            m_GlossinessMapPtr->SetResource(glossinessTexturePtr->GetSRV());
    }

}