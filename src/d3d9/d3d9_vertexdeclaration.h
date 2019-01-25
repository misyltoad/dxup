#pragma once

#include "d3d9_base.h"
#include "d3d9_device_unknown.h"
#include <vector>

namespace dxup {

  class Direct3DVertexDeclaration9 : public D3D9DeviceUnknown<IDirect3DVertexDeclaration9> {

  public:

    Direct3DVertexDeclaration9(Direct3DDevice9Ex* device, std::vector<D3D11_INPUT_ELEMENT_DESC>& d3d11Descs, std::vector<D3DVERTEXELEMENT9>& d3d9Descs)
      : D3D9DeviceUnknown<IDirect3DVertexDeclaration9>{ device }
      , m_d3d11Descs{ d3d11Descs }
      , m_d3d9Descs{ d3d9Descs } { }

    HRESULT STDMETHODCALLTYPE GetDeclaration(D3DVERTEXELEMENT9* pElement, UINT* pNumElements) override {
      if (pNumElements == nullptr)
        return D3DERR_INVALIDCALL;

      if (pElement == nullptr) {
        *pNumElements = m_d3d9Descs.size();
        return D3D_OK;
      }

      UINT size = *pNumElements;
      if (size > m_d3d9Descs.size())
        size = m_d3d9Descs.size();

      std::memcpy(pElement, &m_d3d9Descs[0], (size_t)size);
      return D3D_OK;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
      InitReturnPtr(ppv);

      if (ppv == nullptr)
        return E_POINTER;

      if (riid == __uuidof(IDirect3DVertexDeclaration9) || riid == __uuidof(IUnknown))
        *ppv = ref(this);

      return E_NOINTERFACE;
    }

    const std::vector<D3D11_INPUT_ELEMENT_DESC>& GetD3D11Descs() const {
      return m_d3d11Descs;
    }

  private:
    std::vector<D3D11_INPUT_ELEMENT_DESC> m_d3d11Descs;
    std::vector<D3DVERTEXELEMENT9> m_d3d9Descs;

  };

}