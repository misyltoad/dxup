#include "d3d9_query.h"
#include <algorithm>
#include <Windows.h>

namespace dxup {

  Direct3DQuery9::Direct3DQuery9(Direct3DDevice9Ex* device, D3DQUERYTYPE type)
    : D3D9DeviceUnknown<IDirect3DQuery9>(device)
    , m_type(type) {
    bool doD3D11Query = true;
    switch (m_type) {
    case D3DQUERYTYPE_EVENT: m_d3d11Type = D3D11_QUERY_EVENT; break;
    case D3DQUERYTYPE_OCCLUSION: m_d3d11Type = D3D11_QUERY_OCCLUSION; break;
    case D3DQUERYTYPE_TIMESTAMP: m_d3d11Type = D3D11_QUERY_TIMESTAMP; break;
    case D3DQUERYTYPE_TIMESTAMPDISJOINT: m_d3d11Type = D3D11_QUERY_TIMESTAMP_DISJOINT; break;

    case D3DQUERYTYPE_VCACHE:
    case D3DQUERYTYPE_TIMESTAMPFREQ: 
    default: m_d3d11Type = D3D11_QUERY_EVENT; doD3D11Query = false; break;
    }

    if (doD3D11Query) {
      D3D11_QUERY_DESC desc;
      desc.Query = m_d3d11Type;
      desc.MiscFlags = 0;
      m_device->GetD3D11Device()->CreateQuery(&desc, &m_query);
    }
  }

  HRESULT STDMETHODCALLTYPE Direct3DQuery9::QueryInterface(REFIID riid, LPVOID* ppv) {
    InitReturnPtr(ppv);

    if (!ppv)
      return E_POINTER;

    if (riid == __uuidof(IDirect3DQuery9) || riid == __uuidof(IUnknown)) {
      *ppv = ref(this);
      return D3D_OK;
    }

    return E_NOINTERFACE;
  }

  D3DQUERYTYPE STDMETHODCALLTYPE Direct3DQuery9::GetType() {
    return m_type;
  }
  DWORD STDMETHODCALLTYPE Direct3DQuery9::GetDataSize() {
    switch (m_type) {
    case D3DQUERYTYPE_VCACHE:				return sizeof(D3DDEVINFO_VCACHE);
    case D3DQUERYTYPE_EVENT:				return sizeof(BOOL);
    case D3DQUERYTYPE_OCCLUSION:			return sizeof(DWORD);
    case D3DQUERYTYPE_TIMESTAMP:			return sizeof(UINT64);
    case D3DQUERYTYPE_TIMESTAMPDISJOINT:	return sizeof(BOOL);
    case D3DQUERYTYPE_TIMESTAMPFREQ:		return sizeof(UINT64);
    default: return 0;
    }
  }
  UINT Direct3DQuery9::GetD3D11DataSize() {
    switch (m_d3d11Type) {
    case D3D11_QUERY_EVENT: return sizeof(BOOL);
    default:
    case D3D11_QUERY_OCCLUSION:
    case D3DQUERYTYPE_TIMESTAMPDISJOINT:
    case D3D11_QUERY_TIMESTAMP: return sizeof(UINT64);
    }
  }
  HRESULT STDMETHODCALLTYPE Direct3DQuery9::Issue(DWORD dwIssueFlags) {
    CriticalSection cs(m_device);

    if (dwIssueFlags != D3DISSUE_BEGIN && dwIssueFlags != D3DISSUE_END)
      return D3DERR_INVALIDCALL;

    if (m_query != nullptr) {
      if (dwIssueFlags == D3DISSUE_BEGIN)
        m_device->GetContext()->Begin(m_query.ptr());
      else
        m_device->GetContext()->End(m_query.ptr());
    }

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DQuery9::GetData(void* pData, DWORD dwSize, DWORD dwGetDataFlags) {
    CriticalSection cs(m_device);

    if (pData == nullptr)
      return D3DERR_INVALIDCALL;

    if (dwSize < GetDataSize())
      return D3DERR_INVALIDCALL;

    bool flush = dwGetDataFlags & D3DGETDATA_FLUSH;

    if (m_query != nullptr) {
      uint64_t queryData = 0;
      
      HRESULT result = m_device->GetContext()->GetData(m_query.ptr(), &queryData, GetD3D11DataSize(), flush ? 0 : D3D11_ASYNC_GETDATA_DONOTFLUSH);
      std::memcpy(pData, &queryData, std::min(GetD3D11DataSize(), (size_t)dwSize));

      if (FAILED(result))
        return S_FALSE;
      else
        return D3D_OK;
    }

    if (flush)
      m_device->GetContext()->Flush();

    if (m_type == D3DQUERYTYPE_VCACHE) {
      D3DDEVINFO_VCACHE* cache = (D3DDEVINFO_VCACHE*)pData;
      cache->Pattern = 'CACH';
      cache->OptMethod = 1;
      cache->CacheSize = 16;
      cache->MagicNumber = 8;
      return D3D_OK;
    }

    if (m_type == D3DQUERYTYPE_TIMESTAMPFREQ) {
      QueryPerformanceFrequency((LARGE_INTEGER*)pData);
      return D3D_OK;
    }

    log::warn("Query GetData but I don't know how!");
    return D3DERR_INVALIDCALL;
  }

}