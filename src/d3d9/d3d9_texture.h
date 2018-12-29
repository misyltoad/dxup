#pragma once

#include "d3d9_d3d11_resource.h"
#include "d3d9_resource.h"
#include <vector>

namespace dxup {

  const D3DCUBEMAP_FACES Face0 = (D3DCUBEMAP_FACES)0;

  template <D3DRESOURCETYPE ResourceType, uint32_t slices, typename ID3D9BaseType>
  class Direct3DGeneric2DTexture9 : public Direct3DResource9<ResourceType, ID3D9BaseType> {

  public:

    Direct3DGeneric2DTexture9(bool singletonSurface, Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& desc)
      : Direct3DResource9<ResourceType, ID3D9BaseType>(device, resource, desc)
      , m_singletonSurface{ singletonSurface } {
      m_surfaces.reserve(resource->GetSubresources());

      // This should allow us to match D3D11CalcSubresource.
      for (UINT slice = 0; slice < resource->GetSlices(); slice++) {
        for (UINT mip = 0; mip < resource->GetMips(); mip++) {
          Direct3DSurface9* surface = new Direct3DSurface9(singletonSurface, slice, mip, device, this, resource, desc);

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
      return (DWORD)GetDXUPResource()->GetMips();
    }

    HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
      if (Level >= m_surfaces.size())
        return D3DERR_INVALIDCALL;

      return m_surfaces[Level]->GetDesc(pDesc);
    }

    HRESULT STDMETHODCALLTYPE GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface) {
      UINT subresource = D3D11CalcSubresource(Level, (UINT)FaceType, GetDXUPResource()->GetMips());

      InitReturnPtr(ppCubeMapSurface);
      if (subresource >= m_surfaces.size() || ppCubeMapSurface == nullptr)
        return D3DERR_INVALIDCALL;

      *ppCubeMapSurface = ref(m_surfaces[subresource]);

      return D3D_OK;
    }

    HRESULT STDMETHODCALLTYPE GetSurfaceLevel(UINT Level, IDirect3DSurface9** ppSurfaceLevel) {
      return GetCubeMapSurface((D3DCUBEMAP_FACES)0, Level, ppSurfaceLevel);
    }

    HRESULT STDMETHODCALLTYPE LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags) {
      UINT subresource = D3D11CalcSubresource(Level, (UINT)FaceType, GetDXUPResource()->GetMips());

      if (subresource >= m_surfaces.size())
        return D3DERR_INVALIDCALL;

      return m_surfaces[subresource]->LockRect(pLockedRect, pRect, Flags);
    }

    HRESULT STDMETHODCALLTYPE LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags) {
      return LockRect(Face0, Level, pLockedRect, pRect, Flags);
    }

    HRESULT STDMETHODCALLTYPE UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level) {
      UINT subresource = D3D11CalcSubresource(Level, (UINT)FaceType, GetDXUPResource()->GetMips());

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
      if (GetDXUPResource()->GetSRV() != nullptr)
        m_device->GetContext()->GenerateMips(GetDXUPResource()->GetSRV());
      else
        log::warn("GenerateMipSubLevels called on a texture with no SRV.");
    }

    inline bool IsFakeSurface() {
      return m_singletonSurface;
    }

    DWORD STDMETHODCALLTYPE SetLOD(DWORD LODNew) override {
      log::stub("Direct3DBaseTexture9::SetLOD");
      return 0;
    }
    DWORD STDMETHODCALLTYPE GetLOD() override {
      log::stub("Direct3DBaseTexture9::GetLOD");
      return 0;
    }

    HRESULT STDMETHODCALLTYPE SetAutoGenFilterType(D3DTEXTUREFILTERTYPE FilterType) override {
      log::stub("Direct3DBaseTexture9::SetAutoGenFilterType");
      return D3D_OK;
    }
    D3DTEXTUREFILTERTYPE STDMETHODCALLTYPE GetAutoGenFilterType() override {
      log::stub("Direct3DBaseTexture9::GetAutoGenFilterType");
      return D3DTEXF_ANISOTROPIC;
    }

  private:
    bool m_singletonSurface;
    std::vector<IDirect3DSurface9*> m_surfaces;
  };

  using Direct3DTexture9 = Direct3DGeneric2DTexture9<D3DRTYPE_TEXTURE, 1, IDirect3DTexture9>;
  using Direct3DCubeTexture9 = Direct3DGeneric2DTexture9<D3DRTYPE_CUBETEXTURE, 6, IDirect3DCubeTexture9>;
}