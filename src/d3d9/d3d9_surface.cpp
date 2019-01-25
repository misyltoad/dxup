#include "d3d9_surface.h"
#include "d3d9_format.h"
#include "d3d9_texture.h"
#include "d3d9_format.h"
#include "../util/config.h"

namespace dxup {

  Direct3DSurface9::Direct3DSurface9(bool singletonSurface, UINT slice, UINT mip, Direct3DDevice9Ex* device, IUnknown* container, DXUPResource* resource, const D3D9ResourceDesc& desc)
    : Direct3DSurface9Base(device, nullptr, desc)
    , m_container(container)
    , m_slice(slice)
    , m_mip(mip)
    , m_rtView(nullptr)
    , m_rtViewSRGB(nullptr)
    , m_singletonSurface(singletonSurface)
    , m_useRect(false)
  {
    if (singletonSurface && m_container != nullptr)
        m_container->AddRef();

    this->SetResource(resource);
  }

  Direct3DSurface9::~Direct3DSurface9() {
    if (m_singletonSurface && m_container != nullptr)
      m_container->Release();
  }

  HRESULT Direct3DSurface9::GetContainer(REFIID riid, void** ppContainer) {
    if (ppContainer == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetContainer: ppContainer was nullptr.");

    if (riid == __uuidof(IDirect3DDevice9) || riid == __uuidof(IDirect3DDevice9Ex)) {
      *ppContainer = (void*)ref(m_device);
      return D3D_OK;
    }

    if (m_container == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetContainer: m_container was nullptr.");

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
      return log::d3derr(D3DERR_INVALIDCALL, "GetDesc: pDesc was nullptr.");

    D3D11_TEXTURE2D_DESC desc;
    m_resource->GetResourceAs<ID3D11Texture2D>()->GetDesc(&desc);

    pDesc->Format = m_d3d9Desc.Format;
    pDesc->Height = desc.Height;
    pDesc->Width = desc.Width;
    pDesc->Pool = m_d3d9Desc.Pool;
    pDesc->MultiSampleType = (D3DMULTISAMPLE_TYPE)desc.SampleDesc.Count;
    pDesc->MultiSampleQuality = desc.SampleDesc.Quality;
    pDesc->Type = D3DRTYPE_SURFACE;
    pDesc->Usage = m_d3d9Desc.Usage;

    return D3D_OK;
  }
  HRESULT Direct3DSurface9::LockRect(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) {
    return m_resource->D3D9LockRect(m_slice, m_mip, pLockedRect, pRect, Flags, m_d3d9Desc.Usage);
  }
  HRESULT Direct3DSurface9::UnlockRect() {
    return m_resource->D3D9UnlockRect(m_slice, m_mip);
  }

  D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DSurface9::GetType() {
    return D3DRTYPE_SURFACE;
  }

  HRESULT Direct3DSurface9::GetDC(HDC *phdc) {
    InitReturnPtr(phdc);

    if (!config::getBool(config::GDICompatible))
      return log::d3derr(D3DERR_INVALIDCALL, "GetDC: GDI compatibility not enabled.");

    HRESULT result = D3DERR_INVALIDCALL;
    if (m_surface != nullptr && phdc != nullptr)
      result = m_surface->GetDC(FALSE, phdc);

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "GetDC: failed to get DC.");

    return D3D_OK;
  }
  HRESULT Direct3DSurface9::ReleaseDC(HDC hdc) {
    if (!config::getBool(config::GDICompatible))
      return log::d3derr(D3DERR_INVALIDCALL, "ReleaseDC: GDI compatibility not enabled.");

    HRESULT result = D3DERR_INVALIDCALL;
    if (m_surface != nullptr && hdc != nullptr)
      result = m_surface->ReleaseDC(nullptr);

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "ReleaseDC: failed to release DC.");

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

  IDXGISurface1* Direct3DSurface9::GetDXGISurface() {
    return m_surface.ptr();
  }
  ID3D11RenderTargetView* Direct3DSurface9::GetD3D11RenderTarget(bool srgb) {
    if (srgb && m_rtViewSRGB != nullptr)
      return m_rtViewSRGB.ptr();

    return m_rtView.ptr();
  }
  ID3D11DepthStencilView* Direct3DSurface9::GetD3D11DepthStencil() {
    return m_dsView.ptr();
  }

  UINT Direct3DSurface9::GetMip() {
    return m_mip;
  }
  UINT Direct3DSurface9::GetSlice() {
    return m_slice;
  }
  UINT Direct3DSurface9::GetSubresource() {
    return D3D11CalcSubresource(m_mip, m_slice, this->GetDXUPResource()->GetMips());
  }

  void Direct3DSurface9::ClearResource() {
    m_resource = nullptr;
    m_dsView = nullptr;
    m_rtView = nullptr;
    m_rtViewSRGB = nullptr;
    m_surface = nullptr;
  }
  void Direct3DSurface9::SetResource(DXUPResource* resource) {
    this->ClearResource();

    m_resource = resource;

    if (GetSubresource() == 0 && m_d3d9Desc.Usage & D3DUSAGE_DEPTHSTENCIL) {
      if (FAILED(GetD3D11Device()->CreateDepthStencilView(resource->GetResource(), nullptr, &m_dsView)))
        log::warn("Failed to create depth stencil for surface!");
    }

    if (GetSubresource() == 0 && m_d3d9Desc.Usage & D3DUSAGE_RENDERTARGET) {
      D3D11_RENDER_TARGET_VIEW_DESC desc;
      desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
      desc.Texture2D.MipSlice = 0;

      desc.Format = convert::makeTypeless(resource->GetDXGIFormat());
      desc.Format = convert::makeUntypeless(desc.Format, false);
      if (FAILED(GetD3D11Device()->CreateRenderTargetView(resource->GetResource(), &desc, &m_rtView)))
        log::warn("Failed to create non-SRGB render target for surface!");

      desc.Format = convert::makeTypeless(resource->GetDXGIFormat());
      desc.Format = convert::makeUntypeless(desc.Format, true);
      if (FAILED(GetD3D11Device()->CreateRenderTargetView(resource->GetResource(), &desc, &m_rtViewSRGB)))
        log::warn("Failed to create SRGB render target for surface!");
    }

    if (config::getBool(config::GDICompatible))
      resource->GetResource()->QueryInterface(__uuidof(IDXGISurface1), (void**)&m_surface);
  }
  bool Direct3DSurface9::isRectValid(const RECT* rect) {
    if (rect == nullptr)
      return true;

    if (rect->right <= rect->left || rect->bottom <= rect->top)
      return false;

    if (rect->left < 0 || rect->top < 0)
      return false;

    D3DSURFACE_DESC desc;
    this->GetDesc(&desc);
    if (rect->right > (int)desc.Width || rect->bottom > (int)desc.Height)
      return false;

    return true;
  }

  bool Direct3DSurface9::isBoxValid(const D3D11_BOX* box) {
    if (box == nullptr)
      return true;

    if (box->front != 0 || box->back != 1)
      return false;

    RECT rect;
    rect.left = box->left;
    rect.top = box->top;
    rect.right = box->right;
    rect.bottom = box->bottom;
    return this->isRectValid(&rect);
  }
}