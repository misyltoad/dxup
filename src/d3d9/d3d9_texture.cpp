#include "d3d9_texture.h"
#include "d3d9_surface.h"

namespace dxapex {

  Direct3DTexture9::Direct3DTexture9(Direct3DDevice9Ex* device, ID3D11Texture2D* texture, D3DPOOL pool, DWORD usage)
    : Direct3DTexture9Base(device, texture, pool, usage) {

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    if (desc.Usage != D3D11_USAGE_DYNAMIC) {
      // See comment in d3d9_vertexbuffer.cpp

      desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
      desc.Usage = D3D11_USAGE_STAGING;

      Com<ID3D11Device> device;
      m_device->GetD3D11Device(&device);

      Com<ID3D11Texture2D> stagingTex;
      device->CreateTexture2D(&desc, nullptr, &stagingTex);
      SetStagingResource(stagingTex.ptr());
    }

    m_surfaces.reserve(desc.MipLevels);
    for (UINT i = 0; i < desc.MipLevels; i++)
      m_surfaces.push_back(ref(new Direct3DSurface9(false, i, device, this, texture, pool, usage)));
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
    Com<ID3D11Texture2D> texture;
    GetResource(&texture);

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

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