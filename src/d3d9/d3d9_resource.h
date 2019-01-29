#pragma once

#include "d3d9_base.h"
#include "d3d9_format.h"
#include "d3d9_device_unknown.h"
#include "d3d9_d3d11_resource.h"
#include "../util/private_data_d3d.h"

#include <vector>

namespace dxup {

  struct D3D9ResourceDesc {
    D3DPOOL Pool = D3DPOOL_DEFAULT;
    DWORD Usage = 0;
    BOOL Discard = false;
    D3DFORMAT Format = D3DFMT_UNKNOWN;
    DWORD Priority = 0;
    DWORD FVF = 0;
  };

  template <D3DRESOURCETYPE ResourceType, typename... Base>
  class Direct3DResource9 : public D3D9DeviceUnknown<Base...> {

  public:

    Direct3DResource9(Direct3DDevice9Ex* pDevice, DXUPResource* resource, const D3D9ResourceDesc& d3d9Desc)
      : D3D9DeviceUnknown<Base...>{ pDevice }
      , m_resource{ resource }
      , m_d3d9Desc{ d3d9Desc } {}

    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags) override {
      return m_map.SetPrivateData(refguid, pData, SizeOfData, Flags);
    }
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData) override {
      return m_map.GetPrivateData(refguid, pData, pSizeOfData);
    }
    HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) override {
      return m_map.FreePrivateData(refguid);
    }
    DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) {
      DWORD oldPriority = m_d3d9Desc.Priority;
      m_d3d9Desc.Priority = PriorityNew;
      return oldPriority;
    }
    DWORD STDMETHODCALLTYPE GetPriority() {
      return m_d3d9Desc.Priority;
    }
    void STDMETHODCALLTYPE PreLoad() {
      log::stub("Direct3DResource9::PreLoad");
    }
    D3DRESOURCETYPE STDMETHODCALLTYPE GetType() {
      return ResourceType;
    }

    const D3D9ResourceDesc& GetD3D9Desc() {
      return m_d3d9Desc;
    }

    DXUPResource* GetDXUPResource() {
      return m_resource.ptr();
    }

  protected:

    Com<DXUPResource> m_resource;

    D3D9ResourceDesc m_d3d9Desc;
    PrivateDataD3D m_map;
  };

}