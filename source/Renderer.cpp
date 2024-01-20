#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Utils.h"

namespace dae {

#pragma region Global
	std::vector<Vertex>   vehicle_vertices{};
	std::vector<uint32_t> vehicle_indices{};
	std::vector<Vertex>   fireFx_vertices{};
	std::vector<uint32_t> fireFx_indices{};
#pragma endregion

	Renderer::Renderer(SDL_Window* pWindow) :
		m_WindowPtr(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		// Load Objects
		Utils::ParseOBJ("Resources/vehicle.obj", vehicle_vertices, vehicle_indices);
		Utils::ParseOBJ("Resources/fireFX.obj", fireFx_vertices, fireFx_indices);

		// Create Meshes
		m_MeshPtr = new Mesh(m_DevicePtr, vehicle_vertices, vehicle_indices);
		m_FireFXPtr = new Mesh(m_DevicePtr, fireFx_vertices, fireFx_indices);
		m_FireFXPtr->SetPassIdx(static_cast < UINT>(3));
		
		// Initialize Camera
		const float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };
		m_Camera.SetAspectRatio(aspectRatio);
		m_Camera.Initialize(45.0f, { 0.0f, 0.0f, -50.0f });

		// Load & Set Textures
		m_DiffuseTexturePtr = Texture::LoadFromFile("Resources/vehicle_diffuse.png", m_DevicePtr);
		m_NormalTexturePtr = Texture::LoadFromFile("Resources/vehicle_normal.png", m_DevicePtr);
		m_SpecularTexturePtr = Texture::LoadFromFile("Resources/vehicle_specular.png", m_DevicePtr);
		m_GlossinessTexturePtr = Texture::LoadFromFile("Resources/vehicle_gloss.png", m_DevicePtr);
		m_MeshPtr->SetDiffuseMap(m_DiffuseTexturePtr);
		m_MeshPtr->SetNormalMap(m_NormalTexturePtr);
		m_MeshPtr->SetSpecularMap(m_SpecularTexturePtr);
		m_MeshPtr->SetGlossinessMap(m_GlossinessTexturePtr);

		m_FireFXDiffusePtr = Texture::LoadFromFile("Resources/fireFX_diffuse.png", m_DevicePtr);
		m_FireFXPtr->SetDiffuseMap(m_FireFXDiffusePtr);
	}

	Renderer::~Renderer()
	{
		if (m_RenderTargetViewPtr)   
			m_RenderTargetViewPtr->Release();

		if (m_RenderTargetBufferPtr) 
			m_RenderTargetBufferPtr->Release();

		if (m_DepthStencilViewPtr)   
			m_DepthStencilViewPtr->Release();

		if (m_DepthStencilBufferPtr) 
			m_DepthStencilBufferPtr->Release();

		if (m_SwapChainPtr)          
			m_SwapChainPtr->Release();

		if (m_DeviceContextPtr)
		{
			m_DeviceContextPtr->ClearState();
			m_DeviceContextPtr->Flush();
			m_DeviceContextPtr->Release();
		}

		if (m_DevicePtr)      
			m_DevicePtr->Release();

		if (m_DXGIFactoryPtr) 
			m_DXGIFactoryPtr->Release();

		delete m_MeshPtr;
		delete m_FireFXPtr;

		delete m_DiffuseTexturePtr;
		delete m_GlossinessTexturePtr;
		delete m_NormalTexturePtr;
		delete m_SpecularTexturePtr;
		delete m_FireFXDiffusePtr;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);
		m_MeshPtr->UpdateMatrix(m_Camera.GetInverseViewMatrix(), m_Camera.GetProjectionMatrix());
		m_MeshPtr->SetCameraPosition(m_Camera.GetPosition());
		m_MeshPtr->SetDeltaTime(m_TotalTime);
		m_MeshPtr->SetUseNormalMap(m_UseNormalMap);

		m_FireFXPtr->UpdateMatrix(m_Camera.GetInverseViewMatrix(), m_Camera.GetProjectionMatrix());
		m_FireFXPtr->SetDeltaTime(m_TotalTime);

		if (m_Rotate)
		{
			m_TotalTime += pTimer->GetElapsed();
		}
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		// 1. CLEAR RTV & DSV
		//=======
		const float clearColor[] = { 0.39f, 0.59f, 0.93f, 1.0f };
		m_DeviceContextPtr->ClearRenderTargetView(m_RenderTargetViewPtr, clearColor);
		m_DeviceContextPtr->ClearDepthStencilView(m_DepthStencilViewPtr, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// 2. SET PIPELINE + INVOKE DRAW CALLS (= RENDER)
		//=======
		m_MeshPtr->Render();
		if (m_UseFireFX) m_FireFXPtr->Render();

		// 3. PRESENT BACKBUFFER (SWAP)
		m_SwapChainPtr->Present(0, 0);
	}

	HRESULT Renderer::InitializeDirectX()
	{
		// 1. Create Device & DeviceContext
		//=======
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;

#if defined(DEBUG) or defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result = D3D11CreateDevice(
			nullptr, 
			D3D_DRIVER_TYPE_HARDWARE, 
			0, 
			createDeviceFlags, 
			&featureLevel, 
			1, 
			D3D11_SDK_VERSION, 
			&m_DevicePtr, 
			nullptr, 
			&m_DeviceContextPtr);

		if (FAILED(result))
			return result;

		//Create DXGI factory
		//=======
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&m_DXGIFactoryPtr));
		if (FAILED(result))
			return result;

		// 2. Create swap chain
		//=======
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// Get handle to window (HWND) from the SDL backbuffer
		//=======
		SDL_SysWMinfo sysWMInfo{};
		SDL_GetVersion(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_WindowPtr, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create swap chain
		//=======
		result = m_DXGIFactoryPtr->CreateSwapChain(m_DevicePtr, &swapChainDesc, &m_SwapChainPtr);
		if (FAILED(result))
			return result;

		// 3. Create DepthStencil (DS) and DepthStencilView (DSV)
		//=======
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		// View
		//=======
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_DevicePtr->CreateTexture2D(&depthStencilDesc, nullptr, &m_DepthStencilBufferPtr);
		if (FAILED(result))
			return result;

		result = m_DevicePtr->CreateDepthStencilView(m_DepthStencilBufferPtr, &depthStencilViewDesc, &m_DepthStencilViewPtr);
		if (FAILED(result))
			return result;

		// 4. Create RenderTarget (RT) and RenderTargetView (RTV)
		//=======

		// Resource
		result = m_SwapChainPtr->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_RenderTargetBufferPtr));
		if (FAILED(result))
			return result;

		// View
		result = m_DevicePtr->CreateRenderTargetView(m_RenderTargetBufferPtr, nullptr, &m_RenderTargetViewPtr);
		if (FAILED(result))
			return result;

		// 5. Bind RTV and DSV to Output Merger Stage
		//=======
		m_DeviceContextPtr->OMSetRenderTargets(1, &m_RenderTargetViewPtr, m_DepthStencilViewPtr);

		// 6. Set viewport
		//=======
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_DeviceContextPtr->RSSetViewports(1, &viewport);

		return S_OK;
	}

	void Renderer::CycleSamplerState()
	{
		m_SampleMethod = static_cast<SampleMethod>((static_cast<int>(m_SampleMethod) + 1) % 3);
		m_MeshPtr->SetPassIdx(static_cast<UINT>(m_SampleMethod));
		switch (m_SampleMethod)
		{
		case SampleMethod::Point:
			std::cout << "Current sampling method is Point\n";
			break;
		case SampleMethod::Linear:
			std::cout << "Current sampling method is Linear\n";
			break;
		case SampleMethod::Anisotropic:
			std::cout << "Current sampling method is Anisotropic\n";
			break;
		default:
			std::cout << "Current sampling method is Unknown\n";
			break;
		}
	}
}
