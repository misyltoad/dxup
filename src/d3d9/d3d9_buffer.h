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
        return log::d3derr(D3DERR_INVALIDCALL, "ReturnPtr for buffer lock was null!");

      D3DLOCKED_BOX lockedBox;

      D3DBOX boxToLock;
      boxToLock.Top = 0;
      boxToLock.Bottom = 1;
      boxToLock.Left = OffsetToLock;
      boxToLock.Right = OffsetToLock + SizeToLock;
      boxToLock.Front = 0;
      boxToLock.Back = 1;
      bool degenerate = OffsetToLock == 0 && SizeToLock == 0;

      HRESULT result = this->GetDXUPResource()->D3D9LockBox(0, 0, &lockedBox, degenerate ? nullptr : &boxToLock, Flags, this->GetD3D9Desc().Usage);
      if (FAILED(result))
        return result;

      *ppbData = lockedBox.pBits;

      return D3D_OK;
    }

    HRESULT STDMETHODCALLTYPE Unlock() override {
      return this->GetDXUPResource()->D3D9UnlockBox(0, 0);
    }
  };

  using Direct3DVertexBuffer9Base = Direct3DBuffer9<D3DRTYPE_VERTEXBUFFER, IDirect3DVertexBuffer9>;
  class Direct3DVertexBuffer9 final : public Direct3DVertexBuffer9Base {
  public:
    Direct3DVertexBuffer9(Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& d3d9Desc)
      : Direct3DVertexBuffer9Base{ device, resource, d3d9Desc } { }

    HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC *pDesc) override {
      D3D11_BUFFER_DESC desc;
      this->GetDXUPResource()->GetResourceAs<ID3D11Buffer>()->GetDesc(&desc);

      pDesc->Format = D3DFMT_VERTEXDATA;
      pDesc->FVF = this->GetD3D9Desc().FVF;
      pDesc->Pool = this->GetD3D9Desc().Pool;
      pDesc->Size = desc.ByteWidth;
      pDesc->Type = D3DRTYPE_VERTEXBUFFER;
      pDesc->Usage = this->GetD3D9Desc().Usage;

      return D3D_OK;
    }
  };

  using Direct3DIndexBuffer9Base = Direct3DBuffer9<D3DRTYPE_INDEXBUFFER, IDirect3DIndexBuffer9>;
  class Direct3DIndexBuffer9 final : public Direct3DIndexBuffer9Base {
  public:
    Direct3DIndexBuffer9(Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& d3d9Desc)
      : Direct3DIndexBuffer9Base{ device, resource, d3d9Desc } {}

    HRESULT STDMETHODCALLTYPE GetDesc(D3DINDEXBUFFER_DESC *pDesc) override {
      D3D11_BUFFER_DESC d3d11Desc;
      this->GetDXUPResource()->GetResourceAs<ID3D11Buffer>()->GetDesc(&d3d11Desc);

      const D3D9ResourceDesc& d3d9Desc = this->GetD3D9Desc();

      pDesc->Format = d3d9Desc.Format;
      pDesc->Pool = d3d9Desc.Pool;
      pDesc->Size = d3d11Desc.ByteWidth;
      pDesc->Type = D3DRTYPE_INDEXBUFFER;
      pDesc->Usage = d3d9Desc.Usage;

      return D3D_OK;
    }
  };

}