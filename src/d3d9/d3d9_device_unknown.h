#pragma once

#include "d3d9_base.h"
#include "d3d9_device.h"

namespace dxup {

  template <typename... Base>
  class D3D9DeviceUnknown : public Unknown<Base...> {

  public:

    D3D9DeviceUnknown(Direct3DDevice9Ex* device)
      : m_device{device} {}

    HRESULT STDMETHODCALLTYPE GetDevice(IDirect3DDevice9** ppDevice) override {
      InitReturnPtr(ppDevice);

      if (ppDevice == nullptr)
        return log::d3derr(D3DERR_INVALIDCALL, "GetDevice: ppDevice was nullptr.");
      
      *ppDevice = ref( static_cast<IDirect3DDevice9Ex*>(m_device) );
      return D3D_OK;
    }

    Direct3DDevice9Ex* GetD3D9Device() {
      return m_device;
    }

    ID3D11Device* GetD3D11Device() {
      return m_device->GetD3D11Device();
    }

    ID3D11DeviceContext* GetContext() {
      return m_device->GetContext();
    }

  protected:
    Direct3DDevice9Ex* m_device;
  };

}