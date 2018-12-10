#include "d3d9_vertexbuffer.h"

namespace dxapex {

  Direct3DVertexBuffer9::Direct3DVertexBuffer9(Direct3DDevice9Ex* device, ID3D11Buffer* buffer, D3DPOOL pool, DWORD fvf, DWORD usage, bool d3d11Dynamic)
    : Direct3DVertexBuffer9Base(device, pool, usage, d3d11Dynamic), m_buffer(buffer), m_fvf(fvf) {
    D3D11_BUFFER_DESC desc;
    m_buffer->GetDesc(&desc);

    m_stagingBuffer.resize(DXGI_FORMAT_UNKNOWN, desc.ByteWidth, 1);
  }

  HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer9::QueryInterface(REFIID riid, void** ppvObj) {
    InitReturnPtr(ppvObj);

    if (ppvObj == nullptr)
      return E_POINTER;

    if (riid == __uuidof(IDirect3DVertexBuffer9) || riid == __uuidof(IDirect3DResource9) || riid == __uuidof(IUnknown)) {
      *ppvObj = ref(this);
      return D3D_OK;
    }

    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer9::Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) {
    InitReturnPtr(ppbData);

    if (!ppbData)
      return D3DERR_INVALIDCALL;

    Com<ID3D11DeviceContext> context;
    m_device->GetContext(&context);

    if (IsD3D11Dynamic()) {
      D3D11_MAPPED_SUBRESOURCE resource;
      HRESULT result = context->Map(m_buffer.ptr(), 0, CalcMapType(Flags), CalcMapFlags(Flags), &resource);

      // D3D9 docs say this isn't a thing. I will investigate this later as I don't believe them.
      //if (result == DXGI_ERROR_WAS_STILL_DRAWING)
      //  return D3DERR_WASSTILLDRAWING;

      if (FAILED(result))
        return D3DERR_INVALIDCALL;

      *ppbData = resource.pData;
      
      return D3D_OK;
    }

    // Default path.

    *ppbData = m_stagingBuffer.getDataPtr();

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer9::Unlock() {
    Com<ID3D11DeviceContext> context;
    m_device->GetContext(&context);

    if (IsD3D11Dynamic())
      context->Unmap(m_buffer.ptr(), 0);
    else
      context->UpdateSubresource(m_buffer.ptr(), 0, nullptr, m_stagingBuffer.getDataPtr(), m_stagingBuffer.getPitch(), 0);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer9::GetDesc(D3DVERTEXBUFFER_DESC *pDesc) {
    D3DVERTEXBUFFER_DESC desc;
    desc.Pool = m_pool;
    desc.FVF = m_fvf;

    return D3D_OK;
  }

  void Direct3DVertexBuffer9::GetD3D11Buffer(ID3D11Buffer** buffer) {
    if (buffer != nullptr && m_buffer != nullptr)
      *buffer = ref(m_buffer);
  }

}