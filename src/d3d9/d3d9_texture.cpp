#include "d3d9_texture.h"
#include "d3d9_surface.h"

namespace dxapex {

  Direct3DTexture9::Direct3DTexture9(bool fakeSurface, Direct3DDevice9Ex* device, ID3D11Texture2D* texture, ID3D11ShaderResourceView* srv, D3DPOOL pool, DWORD usage, BOOL discard)
    : Direct3DTexture9Base(device, texture, srv, pool, usage)
    , m_fakeSurface{ fakeSurface }
    , m_mappedSubresources{ 0 }
    , m_unmappedSubresources{ 0 }{

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    if (desc.Usage == D3D11_USAGE_DEFAULT || !(m_usage & D3DUSAGE_WRITEONLY)) {
      // See comment in d3d9_vertexbuffer.cpp
      makeStaging(desc, m_usage);

      Com<ID3D11Texture2D> stagingTex;
      GetD3D11Device()->CreateTexture2D(&desc, nullptr, &stagingTex);
      SetStaging(stagingTex.ptr());
    }

    m_surfaces.reserve(desc.MipLevels);
    for (UINT i = 0; i < desc.MipLevels; i++) {
      Direct3DSurface9* surface = new Direct3DSurface9(false, i, device, this, texture, pool, usage, discard);

      if (!fakeSurface)
        surface->AddRefPrivate();

      m_surfaces.push_back(surface);
    }
  }

  Direct3DTexture9::~Direct3DTexture9() {
    if (!m_fakeSurface) {
      for (IDirect3DSurface9* surface : m_surfaces) {
        Direct3DSurface9* internalSurface = reinterpret_cast<Direct3DSurface9*>(surface);
        internalSurface->ReleasePrivate();
      }
    }
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
    GetResource()->GetDesc(&desc);

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