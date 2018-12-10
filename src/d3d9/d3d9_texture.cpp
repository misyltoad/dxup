#include "d3d9_texture.h"
#include "d3d9_surface.h"

namespace dxapex {

  Direct3DTexture9::Direct3DTexture9(Direct3DDevice9Ex* device, ID3D11Texture2D* texture, D3DPOOL pool, DWORD usage, bool d3d11Dynamic)
    : Direct3DTexture9Base(device, texture, pool, usage, d3d11Dynamic) {

    D3D11_TEXTURE2D_DESC Desc;
    texture->GetDesc(&Desc);

    m_surfaces.reserve(Desc.MipLevels);
    for (UINT i = 0; i < Desc.MipLevels; i++)
      m_surfaces.push_back(ref(new Direct3DSurface9(false, i, device, this, texture, pool, usage, d3d11Dynamic)));
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::QueryInterface(REFIID riid, void** ppvObj) {
    InitReturnPtr(ppvObj);

    if (ppvObj == nullptr)
      return E_POINTER;

    if (riid == __uuidof(IDirect3DTexture9) || riid == __uuidof(IDirect3DResource9) || riid == __uuidof(IUnknown)) {
      *ppvObj = ref(this);
      return D3D_OK;
    }

    return E_NOINTERFACE;
  }

  DWORD STDMETHODCALLTYPE Direct3DTexture9::GetLevelCount() {
    D3D11_TEXTURE2D_DESC desc;
    GetResource<ID3D11Texture2D>()->GetDesc(&desc);

    return desc.MipLevels;
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
    if (Level >= m_surfaces.size())
      return D3DERR_INVALIDCALL;

    return m_surfaces[Level]->GetDesc(pDesc);
  }
  HRESULT STDMETHODCALLTYPE Direct3DTexture9::GetSurfaceLevel(UINT Level, IDirect3DSurface9** ppSurfaceLevel) {
    InitReturnPtr(ppSurfaceLevel);
    if (Level >= m_surfaces.size() || ppSurfaceLevel == nullptr)
      return D3DERR_INVALIDCALL;

    *ppSurfaceLevel = ref(m_surfaces[Level]);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DTexture9::LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags) {
    if (Level >= m_surfaces.size())
      return D3DERR_INVALIDCALL;

    return m_surfaces[Level]->LockRect(pLockedRect, pRect, Flags);
  }
  HRESULT STDMETHODCALLTYPE Direct3DTexture9::UnlockRect(UINT Level) {
    if (Level >= m_surfaces.size())
      return D3DERR_INVALIDCALL;

    return m_surfaces[Level]->UnlockRect();
  }
  HRESULT STDMETHODCALLTYPE Direct3DTexture9::AddDirtyRect(const RECT* pDirtyRect) {
    log::stub("Direct3DTexture9::AddDirtyRect");
    return D3D_OK;
  }

}