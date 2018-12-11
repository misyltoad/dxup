#pragma once

#include "d3d9_resource.h"
#include <vector>

namespace dxapex {

  template <D3DRESOURCETYPE ResourceType, typename Base>
  class Direct3DBuffer9 : public Direct3DResource9<ResourceType, Base>
  {
  public:
    Direct3DBuffer9(Direct3DDevice9Ex* device, ID3D11Buffer* buffer, D3DPOOL pool, DWORD usage) 
      : Direct3DResource9<ResourceType, Base>{ device, pool, usage }, m_buffer{ buffer } {
      D3D11_BUFFER_DESC desc;
      m_buffer->GetDesc(&desc);

      if (desc.Usage == D3D11_USAGE_DEFAULT || !(m_usage & D3DUSAGE_WRITEONLY)) {
        // If we are not a dynamic resource or we need read access on our dynamic, we have a staging buffer.
        // We then map this and CopySubresourceRegion on unmapping.
        // If we don't need the staging res. will be nullptr, some logic relies on this.  

        makeStaging(desc, m_usage);

        Com<ID3D11Device> device;
        m_device->GetD3D11Device(&device);

        device->CreateBuffer(&desc, nullptr, &m_stagingBuffer);
      }
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override {
      InitReturnPtr(ppvObj);

      if (ppvObj == nullptr)
        return E_POINTER;

      if (riid == __uuidof(Base) || riid == __uuidof(IDirect3DResource9) || riid == __uuidof(IUnknown)) {
        *ppvObj = ref(this);
        return D3D_OK;
      }

      return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) override {
      InitReturnPtr(ppbData);

      if (!ppbData)
        return D3DERR_INVALIDCALL;

      Com<ID3D11DeviceContext> context;
      m_device->GetContext(&context);

      Com<ID3D11Buffer> mappedBuffer;
      GetD3D11MappedBuffer(&mappedBuffer);

      D3D11_MAPPED_SUBRESOURCE resource;
      HRESULT result = context->Map(mappedBuffer.ptr(), 0, CalcMapType(m_usage, Flags), CalcMapFlags(Flags), &resource);

      // D3D9 docs say this isn't a thing. I will investigate this later as I don't believe them.
      //if (result == DXGI_ERROR_WAS_STILL_DRAWING)
      //  return D3DERR_WASSTILLDRAWING;

      if (FAILED(result))
        return D3DERR_INVALIDCALL;

      *ppbData = resource.pData;

      return D3D_OK;
    }
    HRESULT STDMETHODCALLTYPE Unlock() override {
      Com<ID3D11DeviceContext> context;
      m_device->GetContext(&context);

      Com<ID3D11Buffer> mappedBuffer;
      GetD3D11MappedBuffer(&mappedBuffer);

      context->Unmap(mappedBuffer.ptr(), 0);

      if (m_stagingBuffer != nullptr)
        context->CopySubresourceRegion(m_buffer.ptr(), 0, 0, 0, 0, m_stagingBuffer.ptr(), 0, nullptr);

      return D3D_OK;
    }

    void GetD3D11MappedBuffer(ID3D11Buffer** buffer) {
      *buffer = ref(m_stagingBuffer);

      if (*buffer == nullptr)
        *buffer = ref(m_buffer); // Dynamic Path
    }

    void GetD3D11Buffer(ID3D11Buffer** buffer) {
      if (buffer != nullptr && m_buffer != nullptr)
        *buffer = ref(m_buffer);
    }

  protected:

    Com<ID3D11Buffer> m_buffer;
    Com<ID3D11Buffer> m_stagingBuffer;
  };

  using Direct3DVertexBuffer9Base = Direct3DBuffer9<D3DRTYPE_VERTEXBUFFER, IDirect3DVertexBuffer9>;
  class Direct3DVertexBuffer9 final : public Direct3DVertexBuffer9Base {
  public:
    Direct3DVertexBuffer9(Direct3DDevice9Ex* device, ID3D11Buffer* buffer, D3DPOOL pool, DWORD fvf, DWORD usage)
      : Direct3DVertexBuffer9Base{ device, buffer, pool, usage }, m_fvf{ fvf } { }

    HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC *pDesc) override {
      D3D11_BUFFER_DESC desc;
      m_buffer->GetDesc(&desc);

      pDesc->Format = D3DFMT_VERTEXDATA;
      pDesc->FVF = m_fvf;
      pDesc->Pool = m_pool;
      pDesc->Size = desc.ByteWidth;
      pDesc->Type = D3DRTYPE_VERTEXBUFFER;
      pDesc->Usage = m_usage;

      return D3D_OK;
    }
  private:
    DWORD m_fvf;
  };

  using Direct3DIndexBuffer9Base = Direct3DBuffer9<D3DRTYPE_INDEXBUFFER, IDirect3DIndexBuffer9>;
  class Direct3DIndexBuffer9 final : public Direct3DIndexBuffer9Base {
  public:
    Direct3DIndexBuffer9(Direct3DDevice9Ex* device, ID3D11Buffer* buffer, D3DPOOL pool, D3DFORMAT format, DWORD usage)
      : Direct3DIndexBuffer9Base{ device, buffer, pool, usage }, m_format{ format } { }

    HRESULT STDMETHODCALLTYPE GetDesc(D3DINDEXBUFFER_DESC *pDesc) override {
      D3D11_BUFFER_DESC desc;
      m_buffer->GetDesc(&desc);

      pDesc->Format = m_format;
      pDesc->Pool = m_pool;
      pDesc->Size = desc.ByteWidth;
      pDesc->Type = D3DRTYPE_INDEXBUFFER;
      pDesc->Usage = m_usage;

      return D3D_OK;
    }

    inline D3DFORMAT GetFormat() {
      return m_format;
    }
  private:
    D3DFORMAT m_format;
  };

}