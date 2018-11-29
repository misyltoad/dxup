#include "d3d9_surface.h"
#include "d3d9_format.h"

namespace dxapex {

  Direct3DSurface9::Direct3DSurface9(bool depthStencil, UINT subresource, Direct3DDevice9Ex* device, IUnknown* container, ID3D11Texture2D* texture, D3DPOOL pool, DWORD usage)
    : Direct3DSurface9Base(device, pool, usage)
    , m_container(container)
    , m_d3d11texture(texture)
    , m_subresource(subresource)
    , m_rtView(nullptr)
    , m_resourceMapPtr(nullptr)
  {
    if (texture != nullptr) {
      D3D11_TEXTURE2D_DESC desc;
      texture->GetDesc(&desc);

      m_surfaceData.resize(calculatePitch(desc.Format, desc.Width) * desc.Height);
    }

    if (m_subresource == 0 && usage & D3DUSAGE_RENDERTARGET) {
      if (texture != nullptr) {
        Com<ID3D11Device> d3d11Device;
        m_device->GetD3D11Device(&d3d11Device);

        HRESULT result = d3d11Device->CreateRenderTargetView(texture, nullptr, &m_rtView);
        if (FAILED(result))
          log::warn("Failed to create render target for surface!");
      }
      else
          log::warn("No D3D11 Texture for Render Target");
    }

    if (texture != nullptr)
      texture->QueryInterface(__uuidof(IDXGISurface1), (void**)&m_surface);
  }

  HRESULT Direct3DSurface9::GetContainer(REFIID riid, void** ppContainer) {
    if (!ppContainer || m_container == nullptr)
      return D3DERR_INVALIDCALL;

    return FAILED(m_container->QueryInterface(riid, ppContainer)) ? D3DERR_INVALIDCALL : D3D_OK;
  }

  HRESULT Direct3DSurface9::QueryInterface(REFIID riid, void** ppvObj) {
    InitReturnPtr(ppvObj);

    if (ppvObj == nullptr)
      return E_POINTER;

    if (riid == __uuidof(IDirect3DSurface9) || riid == __uuidof(IDirect3DResource9) || riid == __uuidof(IUnknown)) {
      *ppvObj = ref(this);
      return D3D_OK;
    }

    return E_NOINTERFACE;
  }

  HRESULT Direct3DSurface9::GetDesc(D3DSURFACE_DESC *pDesc) {
    if (!pDesc)
      return D3DERR_INVALIDCALL;

    Com<ID3D11Texture2D> texture;
    GetD3D11Texture(&texture);

    if (m_surface != nullptr) {
      DXGI_SURFACE_DESC dxgiDesc;
      HRESULT Result = m_surface->GetDesc(&dxgiDesc);

      if (FAILED(Result))
        return Result;

      pDesc->Format = convert::format(dxgiDesc.Format);
      pDesc->Height = dxgiDesc.Height;
      pDesc->Width = dxgiDesc.Width;
      pDesc->Pool = m_pool;
      pDesc->MultiSampleType = (D3DMULTISAMPLE_TYPE)dxgiDesc.SampleDesc.Count;
      pDesc->MultiSampleQuality = dxgiDesc.SampleDesc.Quality;
      pDesc->Type = D3DRTYPE_SURFACE;
      pDesc->Usage = m_usage;
    }
    else if (texture != nullptr) {
      D3D11_TEXTURE2D_DESC desc;
      texture->GetDesc(&desc);
      
      pDesc->Format = convert::format(desc.Format);
      pDesc->Height = desc.Height;
      pDesc->Width = desc.Width;
      pDesc->Pool = m_pool;
      pDesc->MultiSampleType = (D3DMULTISAMPLE_TYPE)desc.SampleDesc.Count;
      pDesc->MultiSampleQuality = desc.SampleDesc.Quality;
      pDesc->Type = D3DRTYPE_SURFACE;
      pDesc->Usage = m_usage;
    }
    else {
      log::warn("Failed to get surface desc, no container and no surface!");
      return D3DERR_INVALIDCALL;
    }

    return D3D_OK;
  }
  HRESULT Direct3DSurface9::LockRect(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) {
    if (!pLockedRect)
      return D3DERR_INVALIDCALL;

    pLockedRect->pBits = nullptr;
    pLockedRect->Pitch = 0;

    if (Flags & D3DLOCK_DISCARD)
      m_surfaceData.clear();

    if (m_d3d11texture != nullptr) {

      D3D11_TEXTURE2D_DESC desc;
      m_d3d11texture->GetDesc(&desc);

      if (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ || desc.Usage & D3D11_USAGE_DYNAMIC) {
        UINT d3d11Flags;
        D3D11_MAP mapType;
        convert::lockFlags(m_pool, Flags, &d3d11Flags, &mapType);

        D3D11_MAPPED_SUBRESOURCE subresource;

        Com<ID3D11DeviceContext> context;
        m_device->GetContext(&context);

        HRESULT result = context->Map(m_d3d11texture.ptr(), m_subresource, mapType, d3d11Flags, &subresource);

        if (result == DXGI_ERROR_WAS_STILL_DRAWING)
          return D3DERR_WASSTILLDRAWING;
        else if (FAILED(result))
          return D3DERR_INVALIDCALL;

        m_surfaceData.resize(subresource.RowPitch * desc.Height);
        std::memcpy(&m_surfaceData[0], subresource.pData, m_surfaceData.size());
        m_resourceMapPtr = subresource.pData;

        pLockedRect->Pitch = subresource.RowPitch;
        m_surfaceDataPitch = subresource.RowPitch;
      }
      else {
        m_resourceMapPtr = nullptr;

        m_surfaceDataPitch = calculatePitch(desc.Format, desc.Width);
        pLockedRect->Pitch = m_surfaceDataPitch;
        m_surfaceData.resize(m_surfaceDataPitch * desc.Height);
      }
      pLockedRect->pBits = &m_surfaceData[0];

      return D3D_OK;
    }

    return D3DERR_INVALIDCALL;
  }
  HRESULT Direct3DSurface9::UnlockRect() {

    if (m_d3d11texture != nullptr) {

      D3D11_TEXTURE2D_DESC desc;
      m_d3d11texture->GetDesc(&desc);

      Com<ID3D11DeviceContext> context;
      m_device->GetContext(&context);

      if (desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ || desc.Usage & D3D11_USAGE_DYNAMIC) {

        if (m_resourceMapPtr != nullptr)
          std::memcpy(m_resourceMapPtr, &m_surfaceData[0], m_surfaceData.size());

        context->Unmap(m_d3d11texture.ptr(), m_subresource);
      }
      else
        context->UpdateSubresource(m_d3d11texture.ptr(), m_subresource, nullptr, &m_surfaceData[0], m_surfaceDataPitch, 0);

    }

    m_resourceMapPtr = nullptr;

    return D3D_OK;
  }

  HRESULT Direct3DSurface9::GetDC(HDC *phdc) {
    InitReturnPtr(phdc);

    if (m_surface != nullptr && phdc != nullptr)
      return m_surface->GetDC(FALSE, phdc);

    return D3DERR_INVALIDCALL;
  }
  HRESULT Direct3DSurface9::ReleaseDC(HDC hdc) {
    if (m_surface != nullptr && hdc != nullptr)
      return m_surface->ReleaseDC(nullptr);

    return D3D_OK;
  }

  void Direct3DSurface9::GetD3D11Texture(ID3D11Texture2D** texture) {
    if (texture != nullptr && m_d3d11texture != nullptr)
      *texture = ref(m_d3d11texture);
  }
  void Direct3DSurface9::GetDXGISurface(IDXGISurface1** surface) {
    if (surface != nullptr && m_surface != nullptr)
      *surface = ref(m_surface);
  }
  void Direct3DSurface9::GetD3D9Texture(Direct3DTexture9** texture) {
    if (texture != nullptr && m_container != nullptr)
      *texture = reinterpret_cast<Direct3DTexture9*>(ref(m_container));
  }

  void Direct3DSurface9::GetD3D11RenderTarget(ID3D11RenderTargetView** rtv) {
    if (rtv != nullptr && m_rtView != nullptr) {
      *rtv = ref(m_rtView);
    }
  }

  UINT Direct3DSurface9::GetSubresource() {
    return m_subresource;
  }

}