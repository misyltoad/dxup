#include "d3d9_vertexbuffer.h"

namespace dxapex {

  Direct3DVertexBuffer9::Direct3DVertexBuffer9(Direct3DDevice9Ex* device, ID3D11Buffer* buffer, D3DPOOL pool, DWORD fvf, DWORD usage)
    : Direct3DVertexBuffer9Base(device, pool, usage), m_buffer(buffer), m_fvf(fvf) {

    D3D11_BUFFER_DESC desc;
    m_buffer->GetDesc(&desc);

    if (desc.Usage != D3D11_USAGE_DYNAMIC) {
      // If we are not a dynamic resource, we have a staging buffer.
      // We then map this and CopySubresourceRegion on unmapping.
      // If it's a dynamic resource the staging equiv. will be nullptr, some logic relies on this.  

      desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
      desc.Usage = D3D11_USAGE_STAGING;

      Com<ID3D11Device> device;
      m_device->GetD3D11Device(&device);

      device->CreateBuffer(&desc, nullptr, &m_stagingBuffer);
    }
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

  void Direct3DVertexBuffer9::GetD3D11MappedBuffer(ID3D11Buffer** buffer) {
    *buffer = ref(m_stagingBuffer);

    if (*buffer == nullptr)
      *buffer = ref(m_buffer); // Dynamic Path
  }

  HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer9::Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) {
    InitReturnPtr(ppbData);

    if (!ppbData)
      return D3DERR_INVALIDCALL;

    Com<ID3D11DeviceContext> context;
    m_device->GetContext(&context);

    Com<ID3D11Buffer> mappedBuffer;
    GetD3D11MappedBuffer(&mappedBuffer);

    D3D11_MAPPED_SUBRESOURCE resource;
    HRESULT result = context->Map(mappedBuffer.ptr(), 0, CalcMapType(Flags), CalcMapFlags(Flags), &resource);

    // D3D9 docs say this isn't a thing. I will investigate this later as I don't believe them.
    //if (result == DXGI_ERROR_WAS_STILL_DRAWING)
    //  return D3DERR_WASSTILLDRAWING;

    if (FAILED(result))
      return D3DERR_INVALIDCALL;

    *ppbData = resource.pData;
      
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer9::Unlock() {
    Com<ID3D11DeviceContext> context;
    m_device->GetContext(&context);

    Com<ID3D11Buffer> mappedBuffer;
    GetD3D11MappedBuffer(&mappedBuffer);

    context->Unmap(mappedBuffer.ptr(), 0);

    if (m_stagingBuffer != nullptr)
      context->CopySubresourceRegion(m_buffer.ptr(), 0, 0, 0, 0, m_stagingBuffer.ptr(), 0, nullptr);

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