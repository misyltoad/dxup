#pragma once

#include "d3d9_resource.h"
#include "d3d9_map_tracker.h"

namespace dxup {

  template <D3DRESOURCETYPE resourceType, typename D3D11ResourceType, typename... ID3D9BaseType>
  class Direct3DBaseTexture9 : public Direct3DResource9<resourceType, ID3D9BaseType...>, public D3D9MapTracker {
  public:
    Direct3DBaseTexture9(uint32_t slices, Direct3DDevice9Ex* device, D3D11ResourceType* resource, ID3D11ShaderResourceView* srv, D3DPOOL pool, DWORD usage)
      : Direct3DResource9<ResourceType, ID3D9BaseType...>(device, pool, usage)
      , m_resource(resource)
      , m_srv{ srv } {}

    DWORD STDMETHODCALLTYPE SetLOD(DWORD LODNew) override {
      log::stub("Direct3DBaseTexture9::SetLOD");
      return 0; s
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

    D3D11ResourceType* GetResource() {
      return m_resource.ptr();
    }

    D3D11ResourceType* GetStaging() {
      return m_stagingResource.ptr();
    }

    ID3D11ShaderResourceView* GetSRV() {
      return m_srv.ptr();
    }

  protected:

    void SetStaging(D3D11ResourceType* res) {
      m_stagingResource = res;
    }

  private:
    Com<D3D11ResourceType> m_resource;
    Com<D3D11ResourceType> m_stagingResource;
    Com<ID3D11ShaderResourceView> m_srv;
  };

}