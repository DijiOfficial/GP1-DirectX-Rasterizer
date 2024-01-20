#include "pch.h"
#include "Texture.h"

using namespace dae;

Texture::Texture(SDL_Surface* pSurface) :
    m_SurfacePtr{ pSurface },
    m_SurfacePixelsPtr{ (uint32_t*)pSurface->pixels }
{

}

Texture::Texture(SDL_Surface* pSurface, ID3D11Device* devicePtr) :
    m_SurfacePtr{ pSurface },
    m_SurfacePixelsPtr{ (uint32_t*)pSurface->pixels }
{
    DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = m_SurfacePtr->w;
    desc.Height = m_SurfacePtr->h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = m_SurfacePtr->pixels;
    initData.SysMemPitch = static_cast<UINT>(m_SurfacePtr->pitch);
    initData.SysMemSlicePitch = static_cast<UINT>(m_SurfacePtr->pitch * m_SurfacePtr->h);

    HRESULT hr = devicePtr->CreateTexture2D(&desc, &initData, &m_ResourcePtr);
    if (FAILED(hr))
    {
        std::cout << "Texture::Texture() failed: " << hr << '\n';
        return;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
    SRVDesc.Format = format;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = desc.MipLevels;

    hr = devicePtr->CreateShaderResourceView(m_ResourcePtr, &SRVDesc, &m_SRVPtr);
    if (FAILED(hr))
    {
        std::cout << "Texture::Texture() failed: " << hr << '\n';
        return;
    }

    if (m_SurfacePtr)
    {
        SDL_FreeSurface(m_SurfacePtr);
        m_SurfacePtr = nullptr;
    }
}

Texture::~Texture()
{
    if (m_SurfacePtr)
    {
        SDL_FreeSurface(m_SurfacePtr);
        m_SurfacePtr = nullptr;
    }

    if (m_ResourcePtr) m_ResourcePtr->Release();
    if (m_SRVPtr) m_SRVPtr->Release();
}

Texture* Texture::LoadFromFile(const std::string& path, ID3D11Device* devicePtr)
{
    SDL_Surface* pSurface = IMG_Load(path.c_str());
    if (!pSurface)
    {
        std::cout << "Texture::LoadFromFile() failed: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    return new Texture(pSurface, devicePtr);
}

ColorRGB Texture::Sample(const Vector2& uv) const
{
    float x{ std::clamp(uv.x, 0.f, 1.f) };
    float y{ std::clamp(uv.y, 0.f, 1.f) };

    x = x * static_cast<float>(m_SurfacePtr->w);
    y = y * static_cast<float>(m_SurfacePtr->h);
    const int index{ static_cast<int>(y) * m_SurfacePtr->w + static_cast<int>(x) };
    const uint32_t pixel{ m_SurfacePixelsPtr[index] };

    uint8_t r, g, b;
    SDL_GetRGB(pixel, m_SurfacePtr->format, &r, &g, &b);
    return ColorRGB{ static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f };
}