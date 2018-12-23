#include "d3d9_surface.h"
#include "d3d9_format.h"
#include "d3d9_texture.h"
#include "d3d9_format.h"

namespace dxup {

  Direct3DSurface9::Direct3DSurface9(bool depthStencil, UINT subresource, Direct3DDevice9Ex* device, IUnknown* container, ID3D11Texture2D* texture, D3DPOOL pool, DWORD usage, BOOL discard, D3DFORMAT format)
    : Direct3DSurface9Base(device, pool, usage)
    , m_container(container)
    , m_d3d11texture(texture)
    , m_subresource(subresource)
    , m_rtView(nullptr)
    , m_discard(discard)
    , m_format(format)
  {
    if (m_subresource == 0 && usage & D3DUSAGE_DEPTHSTENCIL) {
      if (texture != nullptr) {
        HRESULT result = GetD3D11Device()->CreateDepthStencilView(texture, nullptr, &m_dsView);

        if (FAILED(result))
          log::warn("Failed to create depth stencil for surface!");
      }
      else
        log::warn("No D3D11 Texture for Depth Stencil");
    }

    if (m_subresource == 0 && usage & D3DUSAGE_RENDERTARGET) {
      if (texture != nullptr) {
        HRESULT result = GetD3D11Device()->CreateRenderTargetView(texture, nullptr, &m_rtView);

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

    if (m_surface != nullptr) {
      DXGI_SURFACE_DESC dxgiDesc;
      HRESULT Result = m_surface->GetDesc(&dxgiDesc);

      if (FAILED(Result))
        return Result;

      pDesc->Format = m_format;
      pDesc->Height = dxgiDesc.Height;
      pDesc->Width = dxgiDesc.Width;
      pDesc->Pool = m_pool;
      pDesc->MultiSampleType = (D3DMULTISAMPLE_TYPE)dxgiDesc.SampleDesc.Count;
      pDesc->MultiSampleQuality = dxgiDesc.SampleDesc.Quality;
      pDesc->Type = D3DRTYPE_SURFACE;
      pDesc->Usage = m_usage;
    }
    else if (GetD3D11Texture2D() != nullptr) {
      D3D11_TEXTURE2D_DESC desc;
      GetD3D11Texture2D()->GetDesc(&desc);
      
      pDesc->Format = m_format;
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

    if (GetMapping() != nullptr) {

      if (pRect != nullptr)
       m_stagingRect = *pRect;

      m_useRect = pRect != nullptr;

      D3D11_MAPPED_SUBRESOURCE resource;
      HRESULT result = GetContext()->Map(GetMapping(), m_subresource, CalcMapType(Flags), CalcMapFlags(Flags), &resource);

      if (result == DXGI_ERROR_WAS_STILL_DRAWING)
        return D3DERR_WASSTILLDRAWING;

      if (FAILED(result))
        return D3DERR_INVALIDCALL;

      GetD3D9Texture()->SetSubresourceMapped(m_subresource);

      size_t offset = 0;

      if (m_useRect) {
        D3D11_TEXTURE2D_DESC desc;
        GetMapping()->GetDesc(&desc);
        auto& sizeInfo = getDXGIFormatSizeInfo(desc.Format);

        offset = ((pRect->top * resource.RowPitch) + pRect->left) * sizeInfo.pixelBytes / 8;
      }

      uint8_t* data = (uint8_t*)resource.pData;
      pLockedRect->pBits = &data[offset];
      pLockedRect->Pitch = resource.RowPitch;

      return D3D_OK;
    }
    else if (m_surface != nullptr) {
      m_surface->Map((DXGI_MAPPED_RECT*)pLockedRect, CalcMapFlags(Flags));
    }
    else
      log::fail("Surface with no real parent to map to.");

    return D3D_OK;
  }
  HRESULT Direct3DSurface9::UnlockRect() {
    if (GetMapping() != nullptr) {
      GetContext()->Unmap(GetMapping(), m_subresource);
      GetD3D9Texture()->SetSubresourceUnmapped(m_subresource);
    }
    else if (m_surface != nullptr)
      m_surface->Unmap();

    if (HasStaging()) {
      D3D11_BOX box = { 0 };

      if (m_useRect) {
        box.left = m_stagingRect.left;
        box.top = m_stagingRect.top;
        box.right = m_stagingRect.right;
        box.bottom = m_stagingRect.bottom;
        
        box.front = 0;
        box.back = 1;
      }

      if (GetD3D9Texture()->CanPushStaging()) {
        uint64_t delta = GetD3D9Texture()->GetChangedSubresources();
        for (size_t i = 0; i < sizeof(uint64_t) * 8; i++) {
          if ( delta & (1ull << i) )
            GetContext()->CopySubresourceRegion(GetD3D11Texture2D(), i, box.left, box.top, 0, GetStaging(), i, m_useRect ? &box : nullptr);
        }

        GetD3D9Texture()->ResetSubresourceMapInfo();
      }
    }

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

  ULONG STDMETHODCALLTYPE Direct3DSurface9::AddRef() {
    if (m_container != nullptr)
      m_container->AddRef();

    return Direct3DSurface9Base::AddRef();
  }
  ULONG STDMETHODCALLTYPE Direct3DSurface9::Release() {
    if (m_container != nullptr)
      m_container->Release();

    return Direct3DSurface9Base::Release();
  }

  ID3D11Texture2D* Direct3DSurface9::GetMapping() {
    if (HasStaging())
      return GetStaging();

    return GetD3D11Texture2D();
  }
  ID3D11Texture2D* Direct3DSurface9::GetStaging() {
    return GetD3D9Texture()->GetStaging();
  }
  ID3D11Texture2D* Direct3DSurface9::GetD3D11Texture2D() {
    return GetD3D9Texture()->GetResource();
  }
  IDXGISurface1* Direct3DSurface9::GetDXGISurface() {
    return m_surface.ptr();
  }
  Direct3DTexture9* Direct3DSurface9::GetD3D9Texture() {
    return dynamic_cast<Direct3DTexture9*>(m_container);
  }
  ID3D11RenderTargetView* Direct3DSurface9::GetD3D11RenderTarget() {
    return m_rtView.ptr();
  }
  ID3D11DepthStencilView* Direct3DSurface9::GetD3D11DepthStencil() {
    return m_dsView.ptr();
  }

  UINT Direct3DSurface9::GetSubresource() {
    return m_subresource;
  }

  BOOL Direct3DSurface9::GetDiscard() {
    return m_discard;
  }

}