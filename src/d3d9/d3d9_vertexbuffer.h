#pragma once

#include "d3d9_resource.h"
#include <vector>

namespace dxapex {

  using Direct3DVertexBuffer9Base = Direct3DResource9<D3DRTYPE_VERTEXBUFFER, IDirect3DVertexBuffer9>;
  class Direct3DVertexBuffer9 final : public Direct3DVertexBuffer9Base
  {
  public:
    Direct3DVertexBuffer9(Direct3DDevice9Ex* device, ID3D11Buffer* buffer, D3DPOOL pool, DWORD fvf, DWORD usage);

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj);

    HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) override;
    HRESULT STDMETHODCALLTYPE Unlock() override;
    HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC *pDesc) override;

    void GetD3D11Buffer(ID3D11Buffer** buffer);
    void ResizeBuffer();

  private:

    DWORD m_fvf;
    Com<ID3D11Buffer> m_buffer;
    
    std::vector<uint8_t> m_bufferData;
    UINT m_bufferDataPitch;
  };

}