#pragma once
#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	struct Vertex;
	class Texture;
	class Mesh;

	class Renderer final
	{		
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		void CycleSamplerState();
		void ToggleRotation() { m_Rotate = !m_Rotate; std::cout << "Rotation is " << (m_Rotate ? "On" : "Off") << std::endl; };
		void ToggleNormalVisibility() { m_UseNormalMap = !m_UseNormalMap; std::cout << "Normal map is " << (m_UseNormalMap ? "On" : "Off") << std::endl; };
		void ToggleFireFX() { m_UseFireFX = !m_UseFireFX; std::cout << "FireFx is " << (m_UseFireFX ? "On" : "Off") << std::endl; };
	private:
		SDL_Window* m_WindowPtr{};

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };
		bool m_Rotate{ true };
		bool m_UseNormalMap{ true };
		bool m_UseFireFX{ true };

		float m_TotalTime{ 0.f };

		enum class SampleMethod
		{
			Point,
			Linear,
			Anisotropic,
		};
		SampleMethod m_SampleMethod = SampleMethod::Point;

		//DIRECTX
		HRESULT InitializeDirectX();

		IDXGIFactory1* m_DXGIFactoryPtr = nullptr;
		ID3D11Device* m_DevicePtr = nullptr;
		ID3D11DeviceContext* m_DeviceContextPtr = nullptr;
		IDXGISwapChain* m_SwapChainPtr = nullptr;
		ID3D11Texture2D* m_DepthStencilBufferPtr = nullptr;
		ID3D11DepthStencilView* m_DepthStencilViewPtr = nullptr;
		ID3D11Resource* m_RenderTargetBufferPtr = nullptr;
		ID3D11RenderTargetView* m_RenderTargetViewPtr = nullptr;
		
		Camera m_Camera{ };
		Mesh* m_MeshPtr = nullptr;
		Mesh* m_FireFXPtr = nullptr;

		// Vehicle
		Texture* m_DiffuseTexturePtr = nullptr;
		Texture* m_GlossinessTexturePtr = nullptr;
		Texture* m_NormalTexturePtr = nullptr;
		Texture* m_SpecularTexturePtr = nullptr;
		Texture* m_FireFXDiffusePtr = nullptr;
	};
}
