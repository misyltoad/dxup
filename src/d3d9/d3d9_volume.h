#pragma once

#include "d3d9_resource.h"
#include "d3d9_d3d11_resource.h"
#include <vector>

namespace dxup {

  using Direct3DVolume9Base = Direct3DResource9<D3DRTYPE_VOLUME, IDirect3DVolume9>;

  // A d3d9 surface is essentially some subresource of a d3d11 texture.
  class Direct3DVolume9 final : public Direct3DVolume9Base {
    
  public:

    HRESULT WINAPI QueryInterface(REFIID riid, LPVOID* ppv) override;
    HRESULT WINAPI GetContainer(REFIID riid, void** ppContainer) override;
    HRESULT WINAPI GetDesc(D3DVOLUME_DESC *pDesc) override;
    HRESULT WINAPI LockBox(D3DLOCKED_BOX* pLockedBox, CONST D3DBOX* pBox, DWORD Flags) override;
    HRESULT WINAPI UnlockBox() override;

    UINT GetMip();
    UINT GetSubresource();

    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

    void ClearResource();
    void SetResource(DXUPResource* resource);

    static Direct3DVolume9* Wrap(UINT mip, Direct3DDevice9Ex* device, IUnknown* container, DXUPResource* resource, const D3D9ResourceDesc& desc);

  private:

    Direct3DVolume9(UINT mip, Direct3DDevice9Ex* device, IUnknown* container, DXUPResource* resource, const D3D9ResourceDesc& desc);

    IUnknown* m_container;

    UINT m_mip;
  };

}