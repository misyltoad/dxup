#pragma once

#include "d3d9_base.h"
#include "d3d9_device_unknown.h"

namespace dxup {

  class Direct3DQuery9 : public D3D9DeviceUnknown<IDirect3DQuery9> {

  public:

    Direct3DQuery9(Direct3DDevice9Ex* device, D3DQUERYTYPE type);

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) override;

    D3DQUERYTYPE STDMETHODCALLTYPE GetType() override;
    DWORD STDMETHODCALLTYPE GetDataSize() override;
    HRESULT STDMETHODCALLTYPE Issue(DWORD dwIssueFlags) override;
    HRESULT STDMETHODCALLTYPE GetData(void* pData, DWORD dwSize, DWORD dwGetDataFlags) override;

  private:

    UINT GetD3D11DataSize();

    D3DQUERYTYPE m_type;
    D3D11_QUERY m_d3d11Type;
    Com<ID3D11Query> m_query;
  };

}