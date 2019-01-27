#include "d3d9_texture.h"
#include "d3d9_device.h"

namespace dxup {

  // Direct3DTexture9

  Direct3DTexture9::Direct3DTexture9(bool singletonSurface, Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& desc)
    : Direct3DTexture9Base{ device, resource, desc }
    , m_singletonSurface{ singletonSurface } {
    m_surfaces.reserve(resource->GetSubresources());

    for (UINT mip = 0; mip < resource->GetMips(); mip++) {
      Direct3DSurface9* surface = new Direct3DSurface9(singletonSurface, 0, mip, device, this, resource, desc);

      if (!singletonSurface)
        surface->AddRefPrivate();

      m_surfaces.push_back(surface);
    }
  }

  Direct3DTexture9::~Direct3DTexture9() {
    if (!m_singletonSurface) {
      for (IDirect3DSurface9* surface : m_surfaces) {
        Direct3DSurface9* internalSurface = reinterpret_cast<Direct3DSurface9*>(surface);
        internalSurface->ReleasePrivate();
      }
    }
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
    CriticalSection(this->GetD3D9Device());

    if (Level >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetLevelDesc: level out of bounds.");

    return m_surfaces[Level]->GetDesc(pDesc);
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::GetSurfaceLevel(UINT Level, IDirect3DSurface9** ppSurfaceLevel) {
    CriticalSection(this->GetD3D9Device());

    UINT subresource = D3D11CalcSubresource(Level, 0, this->GetDXUPResource()->GetMips());

    InitReturnPtr(ppSurfaceLevel);

    if (subresource >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetSurfaceLevel: subresource out of bounds (Level = %d).", Level);

    if (ppSurfaceLevel == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetSurfaceLevel: ppSurfaceLevel was nullptr.");

    *ppSurfaceLevel = ref(m_surfaces[subresource]);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags) {
    CriticalSection(this->GetD3D9Device());

    UINT subresource = D3D11CalcSubresource(Level, 0, this->GetDXUPResource()->GetMips());

    if (subresource >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "LockRect: subresource out of bounds (Level = %d).", Level);

    return m_surfaces[subresource]->LockRect(pLockedRect, pRect, Flags);
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::UnlockRect(UINT Level) {
    CriticalSection(this->GetD3D9Device());

    UINT subresource = D3D11CalcSubresource(Level, 0, this->GetDXUPResource()->GetMips());

    if (subresource >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "UnlockRect: subresource out of bounds (Level = %d).", Level);

    return m_surfaces[subresource]->UnlockRect();
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::AddDirtyRect(const RECT* pDirtyRect) {
    CriticalSection(this->GetD3D9Device());
    log::stub("Direct3DTexture9::AddDirtyRect");

    return D3D_OK;
  }

  inline bool Direct3DTexture9::IsFakeSurface() {
    return m_singletonSurface;
  }

  // Direct3DCubeTexture9

  Direct3DCubeTexture9::Direct3DCubeTexture9(Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& desc)
    : Direct3DCubeTexture9Base{ device, resource, desc } {
    m_surfaces.reserve(resource->GetSubresources());

    // This should allow us to match D3D11CalcSubresource.
    for (UINT slice = 0; slice < 6; slice++) {
      for (UINT mip = 0; mip < resource->GetMips(); mip++) {
        Direct3DSurface9* surface = new Direct3DSurface9(false, slice, mip, device, this, resource, desc);
        surface->AddRefPrivate();

        m_surfaces.push_back(surface);
      }
    }
  }

  Direct3DCubeTexture9::~Direct3DCubeTexture9() {
    for (IDirect3DSurface9* surface : m_surfaces) {
      Direct3DSurface9* internalSurface = reinterpret_cast<Direct3DSurface9*>(surface);
      internalSurface->ReleasePrivate();
    }
  }

  HRESULT STDMETHODCALLTYPE Direct3DCubeTexture9::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
    CriticalSection(this->GetD3D9Device());

    if (Level >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetLevelDesc: level out of bounds.");

    return m_surfaces[Level]->GetDesc(pDesc);
  }

  HRESULT STDMETHODCALLTYPE Direct3DCubeTexture9::GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface) {
    CriticalSection(this->GetD3D9Device());

    UINT subresource = D3D11CalcSubresource(Level, (UINT)FaceType, this->GetDXUPResource()->GetMips());

    InitReturnPtr(ppCubeMapSurface);

    if (subresource >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetCubeMapSurface: subresource out of bounds (FaceType = %d, Level = %d).", FaceType, Level);

    if (ppCubeMapSurface == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetCubeMapSurface: ppCubeMapSurface was nullptr.");

    *ppCubeMapSurface = ref(m_surfaces[subresource]);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DCubeTexture9::LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags) {
    CriticalSection(this->GetD3D9Device());

    UINT subresource = D3D11CalcSubresource(Level, FaceType, this->GetDXUPResource()->GetMips());

    if (subresource >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "LockRect: subresource out of bounds (Level = %d).", Level);

    return m_surfaces[subresource]->LockRect(pLockedRect, pRect, Flags);
  }

  HRESULT STDMETHODCALLTYPE Direct3DCubeTexture9::UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level) {
    CriticalSection(this->GetD3D9Device());

    UINT subresource = D3D11CalcSubresource(Level, (UINT)FaceType, this->GetDXUPResource()->GetMips());

    if (subresource >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "UnlockRect: subresource out of bounds (FaceType = %d, Level = %d).", FaceType, Level);

    return m_surfaces[subresource]->UnlockRect();
  }

  HRESULT STDMETHODCALLTYPE Direct3DCubeTexture9::AddDirtyRect(D3DCUBEMAP_FACES FaceType, const RECT* pDirtyRect) {
    CriticalSection(this->GetD3D9Device());
    log::stub("Direct3DCubeTexture9::AddDirtyRect");

    return D3D_OK;
  }
}