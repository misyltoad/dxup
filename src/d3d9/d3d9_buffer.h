#pragma once

#include "d3d9_resource.h"
#include "d3d9_d3d11_resource.h"
#include <vector>

namespace dxup {

  template <D3DRESOURCETYPE ResourceType, typename ID3D9BaseType>
  class Direct3DBuffer9 : public Direct3DResource9<ResourceType, ID3D9BaseType>
  {
  public:
    Direct3DBuffer9(Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& d3d9Desc)
      : Direct3DResource9<ResourceType, ID3D9BaseType>{ device, resource, d3d9Desc } {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override {
      InitReturnPtr(ppvObj);

      if (ppvObj == nullptr)
        return E_POINTER;

      if (riid == __uuidof(ID3D9BaseType) || riid == __uuidof(IDirect3DResource9) || riid == __uuidof(IUnknown)) {
        *ppvObj = ref(this);
        return D3D_OK;
      }

      return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) override {
      InitReturnPtr(ppbData);
      if (ppbData == nullptr)
        return D3DERR_INVALIDCALL;

      D3DLOCKED_RECT lockedRect;

      RECT rectToLock;
      rectToLock.bottom = 0;
      rectToLock.top = 0;
      rectToLock.left = OffsetToLock;
      rectToLock.right = SizeToLock;
      bool degenerate = isRectDegenerate(rectToLock);

      HRESULT result = m_resource->D3D9LockRect(0, 0, &lockedRect, degenerate ? nullptr : &rectToLock, Flags, m_d3d9Desc.Usage);
      if (FAILED(result))
        return result;

      *ppbData = lockedRect.pBits;

      return D3D_OK;
    }

    HRESULT STDMETHODCALLTYPE Unlock() override {
      return m_resource->D3D9UnlockRect(0, 0);
    }
  };

  using Direct3DVertexBuffer9Base = Direct3DBuffer9<D3DRTYPE_VERTEXBUFFER, IDirect3DVertexBuffer9>;
  class Direct3DVertexBuffer9 final : public Direct3DVertexBuffer9Base {
  public:
    Direct3DVertexBuffer9(Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& d3d9Desc)
      : Direct3DVertexBuffer9Base{ device, resource, d3d9Desc } { }

    HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC *pDesc) override {
      D3D11_BUFFER_DESC desc;
      m_resource->GetResourceAs<ID3D11Buffer>()->GetDesc(&desc);

      pDesc->Format = D3DFMT_VERTEXDATA;
      pDesc->FVF = m_d3d9Desc.FVF;
      pDesc->Pool = m_d3d9Desc.Pool;
      pDesc->Size = desc.ByteWidth;
      pDesc->Type = D3DRTYPE_VERTEXBUFFER;
      pDesc->Usage = m_d3d9Desc.Usage;

      return D3D_OK;
    }
  };

  using Direct3DIndexBuffer9Base = Direct3DBuffer9<D3DRTYPE_INDEXBUFFER, IDirect3DIndexBuffer9>;
  class Direct3DIndexBuffer9 final : public Direct3DIndexBuffer9Base {
  public:
    Direct3DIndexBuffer9(Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& d3d9Desc)
      : Direct3DIndexBuffer9Base{ device, resource, d3d9Desc } {}

    HRESULT STDMETHODCALLTYPE GetDesc(D3DINDEXBUFFER_DESC *pDesc) override {
      D3D11_BUFFER_DESC desc;
      m_resource->GetResourceAs<ID3D11Buffer>()->GetDesc(&desc);

      pDesc->Format = m_d3d9Desc.Format;
      pDesc->Pool = m_d3d9Desc.Pool;
      pDesc->Size = desc.ByteWidth;
      pDesc->Type = D3DRTYPE_INDEXBUFFER;
      pDesc->Usage = m_d3d9Desc.Usage;

      return D3D_OK;
    }
  };

}