#pragma once

#include "d3d9_base.h"
#include "d3d9_format.h"
#include "d3d9_device_unknown.h"
#include "../util/private_data_d3d.h"

#include <vector>

namespace dxup {

  template <D3DRESOURCETYPE ResourceType, typename... Base>
  class Direct3DResource9 : public D3D9DeviceUnknown<Base...> {

  public:

    Direct3DResource9(Direct3DDevice9Ex* pDevice, D3DPOOL pool, DWORD usage)
      : D3D9DeviceUnknown<Base...>(pDevice)
      , m_pool(pool)
      , m_priority(0)
      , m_usage(usage) {}

    HRESULT WINAPI SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags) override {
      return m_map.SetPrivateData(refguid, pData, SizeOfData, Flags);
    }
    HRESULT WINAPI GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData) override {
      return m_map.GetPrivateData(refguid, pData, pSizeOfData);
    }
    HRESULT WINAPI FreePrivateData(REFGUID refguid) override {
      return m_map.FreePrivateData(refguid);
    }
    DWORD WINAPI SetPriority(DWORD PriorityNew) override {
      DWORD oldPriority = m_priority;
      m_priority = PriorityNew;
      return oldPriority;
    }
    DWORD WINAPI GetPriority() override {
      return m_priority;
    }
    void WINAPI PreLoad() override {
      log::stub("Direct3DResource9::PreLoad");
    }
    D3DRESOURCETYPE WINAPI GetType() override {
      return ResourceType;
    }

    UINT CalcMapFlags(UINT d3d9LockFlags) {
      return d3d9LockFlags & D3DLOCK_DONOTWAIT ? D3D11_MAP_FLAG_DO_NOT_WAIT : 0;
    }

    D3D11_MAP CalcMapType(UINT d3d9LockFlags) {
      if (m_usage & D3DUSAGE_DYNAMIC) {
        if (d3d9LockFlags & D3DLOCK_NOOVERWRITE)
          return D3D11_MAP_WRITE_NO_OVERWRITE;

        if (d3d9LockFlags & D3DLOCK_DISCARD)
          return D3D11_MAP_WRITE_DISCARD;
      }

      if (d3d9LockFlags & D3DLOCK_READONLY)
        return D3D11_MAP_READ;

      if (m_usage & D3DUSAGE_WRITEONLY)
        return D3D11_MAP_WRITE;

      return D3D11_MAP_READ_WRITE;
    }

  protected:

    DWORD m_priority;
    D3DPOOL m_pool;
    PrivateDataD3D m_map;
    UINT m_usage;
  };

}