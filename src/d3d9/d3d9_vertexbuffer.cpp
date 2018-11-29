#include "d3d9_vertexbuffer.h"

namespace dxapex {

  Direct3DVertexBuffer9::Direct3DVertexBuffer9(Direct3DDevice9Ex* device, ID3D11Buffer* buffer, D3DPOOL pool, DWORD fvf, DWORD usage)
    : Direct3DVertexBuffer9Base(device, pool, usage), m_buffer(buffer), m_fvf(fvf), m_bufferDataPitch(0) {

    ResizeBuffer();
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

    if (Flags & D3DLOCK_DISCARD)
      m_bufferData.clear();

    ResizeBuffer();

    *ppbData = &m_bufferData[0] + OffsetToLock;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer9::Unlock() {
    Com<ID3D11DeviceContext> context;
    m_device->GetContext(&context);

    D3D11_BUFFER_DESC desc;
    m_buffer->GetDesc(&desc);
    context->UpdateSubresource(m_buffer.ptr(), 0, NULL, &m_bufferData[0], desc.ByteWidth, 0);

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

  void Direct3DVertexBuffer9::ResizeBuffer() {
    D3D11_BUFFER_DESC desc;
    m_buffer->GetDesc(&desc);
    m_bufferData.resize(desc.ByteWidth);
  }

}