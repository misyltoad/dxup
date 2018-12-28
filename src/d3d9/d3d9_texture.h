#pragma once

#include "d3d9_basetexture.h"
#include <vector>

namespace dxup {

  const D3DCUBEMAP_FACES Face0 = (D3DCUBEMAP_FACES)0;

  template <D3DRESOURCETYPE resourceType, uint32_t slices, typename ID3D9BaseType>
  class Direct3DGeneric2DTexture9 : public Direct3DBaseTexture9<resourceType, ID3D11Texture2D, ID3D9BaseType> {

  public:

    Direct3DGeneric2DTexture9(bool fakeSurface, Direct3DDevice9Ex* device, ID3D11Texture2D* texture, ID3D11ShaderResourceView* srv, D3DPOOL pool, DWORD usage, BOOL discard, D3DFORMAT format)
      : Direct3DBaseTexture9<ID3D11Texture2D, ID3D9BaseType>(slices, device, texture, srv, pool, usage)
      , m_singletonSurface{ singletonSurface } {

      D3D11_TEXTURE2D_DESC desc;
      texture->GetDesc(&desc);

      m_mips = desc.MipLevels;

      if (desc.Usage == D3D11_USAGE_DEFAULT || !(m_usage & D3DUSAGE_WRITEONLY)) {
        // See comment in d3d9_vertexbuffer.cpp
        makeStaging(desc, m_usage);

        Com<ID3D11Texture2D> stagingTex;
        GetD3D11Device()->CreateTexture2D(&desc, nullptr, &stagingTex);
        SetStaging(stagingTex.ptr());
      }

      m_surfaces.reserve(desc.MipLevels * slices);

      // This should allow us to match D3D11CalcSubresource.
      for (UINT slice = 0; slice < slices; slice++) {
        for (UINT mip = 0; mip < desc.MipLevels; mip++) {
          Direct3DSurface9* surface = new Direct3DSurface9(singletonSurface, slice, mip, device, this, texture, pool, usage, discard, format);

          if (!singletonSurface)
            surface->AddRefPrivate();

          m_surfaces.push_back(surface);
        }
      }
    }

    ~Direct3DGeneric2DTexture9() {
      if (!m_singletonSurface) {
        for (IDirect3DSurface9* surface : m_surfaces) {
          Direct3DSurface9* internalSurface = reinterpret_cast<Direct3DSurface9*>(surface);
          internalSurface->ReleasePrivate();
        }
      }
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) {
      InitReturnPtr(ppvObj);

      if (ppvObj == nullptr)
        return E_POINTER;

      if (riid == __uuidof(ID3D9BaseType) || riid == __uuidof(IDirect3DResource9) || riid == __uuidof(IUnknown)) {
        *ppvObj = ref(this);
        return D3D_OK;
      }

      return E_NOINTERFACE;
    }

    DWORD STDMETHODCALLTYPE GetLevelCount() {
      return (DWORD)m_mips;
    }

    HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
      if (Level >= m_surfaces.size())
        return D3DERR_INVALIDCALL;

      return m_surfaces[Level]->GetDesc(pDesc);
    }

    HRESULT STDMETHODCALLTYPE GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface) {
      UINT subresource = D3D11CalcSubresource(Level, (UINT)FaceType, m_mips);

      InitReturnPtr(ppSurfaceLevel);
      if (subresource >= m_surfaces.size() || ppSurfaceLevel == nullptr)
        return D3DERR_INVALIDCALL;

      *ppSurfaceLevel = ref(m_surfaces[subresource]);

      return D3D_OK;
    }

    HRESULT STDMETHODCALLTYPE GetSurfaceLevel(UINT Level, IDirect3DSurface9** ppSurfaceLevel) {
      return GetCubeMapSurface((D3DCUBEMAP_FACES)0, Level, ppSurfaceLevel);
    }

    HRESULT STDMETHODCALLTYPE LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags) {
      UINT subresource = D3D11CalcSubresource(Level, (UINT)FaceType, m_mips);

      if (subresource >= m_surfaces.size())
        return D3DERR_INVALIDCALL;

      return m_surfaces[subresource]->LockRect(pLockedRect, pRect, Flags);
    }

    HRESULT STDMETHODCALLTYPE LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags) {
      return LockRect(Face0, Level, pLockedRect, pRect, Flags);
    }

    HRESULT STDMETHODCALLTYPE UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level) {
      UINT subresource = D3D11CalcSubresource(Level, (UINT)FaceType, m_mips);

      if (subresource >= m_surfaces.size())
        return D3DERR_INVALIDCALL;

      return m_surfaces[subresource]->UnlockRect();
    }

    HRESULT STDMETHODCALLTYPE UnlockRect(UINT Level) {
      return UnlockRect(Face0, Level);
    }

    HRESULT STDMETHODCALLTYPE AddDirtyRect(D3DCUBEMAP_FACES FaceType, const RECT* pDirtyRect) {
      log::stub("Direct3DTexture9::AddDirtyRect");
      return D3D_OK;
    }

    HRESULT STDMETHODCALLTYPE AddDirtyRect(const RECT* pDirtyRect) {
      return AddDirtyRect(Face0, pDirtyRect);
    }

    void STDMETHODCALLTYPE GenerateMipSubLevels() {
      if (GetSRV() != nullptr)
        m_device->GetContext()->GenerateMips(GetSRV());
      else
        log::warn("GenerateMipSubLevels called on a texture with no SRV.");
    }

    D3DRESOURCETYPE STDMETHODCALLTYPE GetType() {
      return resourceType;
    }

    inline bool IsFakeSurface() {
      return m_singletonSurface;
    }

  private:
    bool m_singletonSurface;
    UINT m_mips;
    std::vector<IDirect3DSurface9*> m_surfaces;
  };

  using Direct3DTexture9 = Direct3DGeneric2DTexture9<D3DRTYPE_TEXTURE, 1, IDirect3DTexture9>;
  using Direct3DCubeTexture9 = Direct3DGeneric2DTexture9<D3DRTYPE_CUBETEXTURE, 6, IDirect3DCubeTexture9>;
}