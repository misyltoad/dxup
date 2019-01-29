#pragma once

#include "d3d9_base.h"
#include "d3d9_device.h"

namespace dxup {

  class DXUPResource : public Unknown<IUnknown> {

  public:

    static DXUPResource* Create(Direct3DDevice9Ex* device, ID3D11Resource* resource, DWORD d3d9Usage, D3DFORMAT d3d9Format);

    bool HasStaging();

    template <typename T>
    T* GetResourceAs() {
      return reinterpret_cast<T*>(m_resource.ptr());
    }

    template <typename T>
    T* GetStagingAs() {
      return reinterpret_cast<T*>(m_staging.ptr());
    }

    template <typename T>
    T* GetMappingAs() {
      if (HasStaging())
        return GetStagingAs<T>();

      return GetResourceAs<T>();
    }

    ID3D11Resource* GetResource();
    ID3D11Resource* GetStaging();
    ID3D11Resource* GetMapping();

    ID3D11ShaderResourceView* GetSRV(bool srgb);

    UINT GetSlices();
    UINT GetMips();
    UINT GetSubresources();

    void SetMipMapped(UINT slice, UINT mip);
    void SetMipUnmapped(UINT slice, UINT mip);
    void MarkDirty(UINT slice, UINT mip);
    void MakeClean();

    uint64_t GetChangedMips(UINT slice);

    bool CanPushStaging();

    void ResetMipMapTracking();

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;

    HRESULT D3D9LockBox(UINT slice, UINT mip, D3DLOCKED_BOX* pLockedBox, CONST D3DBOX* pBox, DWORD Flags, DWORD Usage);
    HRESULT D3D9UnlockBox(UINT slice, UINT mip);

    DXGI_FORMAT GetDXGIFormat();

  private:

    UINT CalcMapFlags(UINT d3d9LockFlags);

    D3D11_MAP CalcMapType(UINT d3d9LockFlags, DWORD d3d9Usage);

    void PushStaging();

    Direct3DDevice9Ex* m_device;

    static bool NeedsStaging(D3D11_USAGE d3d11Usage, DWORD d3d9Usage, D3DFORMAT d3d9Format);
    static DXUPResource* CreateTexture2D(Direct3DDevice9Ex* device, ID3D11Texture2D* texture, DWORD d3d9Usage, D3DFORMAT d3d9Format);
    static DXUPResource* CreateTexture3D(Direct3DDevice9Ex* device, ID3D11Texture3D* texture, DWORD d3d9Usage, D3DFORMAT d3d9Format);
    static DXUPResource* CreateBuffer(Direct3DDevice9Ex* device, ID3D11Buffer* buffer, DWORD d3d9Usage);

    DXUPResource(Direct3DDevice9Ex* device, ID3D11Resource* resource, ID3D11Resource* staging, ID3D11Resource* fixup8888, ID3D11ShaderResourceView* srv, ID3D11ShaderResourceView* srvSRGB, DXGI_FORMAT dxgiFormat, UINT slices, UINT mips, bool dynamic);

    UINT m_slices;
    UINT m_mips;
    DXGI_FORMAT m_dxgiFormat;

    uint64_t m_mappedSubresources[6];
    uint64_t m_unmappedSubresources[6];
    uint64_t m_dirtySubresources[6];

    Com<ID3D11Resource> m_resource;
    Com<ID3D11Resource> m_staging;
    Com<ID3D11Resource> m_fixup8888;

    bool IsStagingBoxDegenerate(UINT subresource);
    std::vector<D3DBOX> m_stagingBoxes;

    bool m_dynamic;

    Com<ID3D11ShaderResourceView> m_srv;
    Com<ID3D11ShaderResourceView> m_srvSRGB;

  };

}