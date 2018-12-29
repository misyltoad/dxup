#include "d3d9_surface.h"
#include "d3d9_format.h"
#include "d3d9_texture.h"
#include "d3d9_format.h"
#include "../util/config.h"

namespace dxup {

  Direct3DSurface9::Direct3DSurface9(bool singletonSurface, UINT slice, UINT mip, Direct3DDevice9Ex* device, IUnknown* container, DXUPResource* resource, const D3D9ResourceDesc& desc)
    : Direct3DSurface9Base(device, resource, desc)
	, m_resource(resource)
    , m_container(container)
    , m_slice(slice)
    , m_mip(mip)
    , m_rtView(nullptr)
    , m_singletonSurface(singletonSurface)
  {
    if (singletonSurface && m_container != nullptr)
        m_container->AddRef();

    if (GetSubresource() == 0 && desc.Usage & D3DUSAGE_DEPTHSTENCIL) {
      if (FAILED(GetD3D11Device()->CreateDepthStencilView(resource->GetResource(), nullptr, &m_dsView)))
        log::warn("Failed to create depth stencil for surface!");
    }

    if (GetSubresource() == 0 && desc.Usage & D3DUSAGE_RENDERTARGET) {
      if (FAILED(GetD3D11Device()->CreateRenderTargetView(resource->GetResource(), nullptr, &m_rtView)))
        log::warn("Failed to create render target for surface!");
    }

    if (config::getBool(config::GDICompatible))
        resource->GetResource()->QueryInterface(__uuidof(IDXGISurface1), (void**)&m_surface);
  }

  Direct3DSurface9::~Direct3DSurface9() {
    if (m_singletonSurface && m_container != nullptr)
      m_container->Release();
  }

  HRESULT Direct3DSurface9::GetContainer(REFIID riid, void** ppContainer) {
    if (!ppContainer || m_container == nullptr)
      return D3DERR_INVALIDCALL;

    if (riid == __uuidof(IDirect3DDevice9) || riid == __uuidof(IDirect3DDevice9Ex)) {
      *ppContainer = (void*)ref(m_device);
      return D3D_OK;
    }

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
    return m_resource->D3D9LockRect(m_slice, m_mip, pLockedRect, pRect, Flags);
  }
  HRESULT Direct3DSurface9::UnlockRect() {
    return m_resource->D3D9UnlockRect(m_slice, m_mip);
  }

  D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DSurface9::GetType() {
    return D3DRTYPE_SURFACE;
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

  IDXGISurface1* Direct3DSurface9::GetDXGISurface() {
    return m_surface.ptr();
  }
  ID3D11RenderTargetView* Direct3DSurface9::GetD3D11RenderTarget() {
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
    return D3D11CalcSubresource(m_mip, m_slice, m_totalMips);
  }

  DXUPResource* Direct3DSurface9::GetDXUPResource() {
    return m_resource;
  }

}