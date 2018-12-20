#include <stdio.h>
#include "d3d9_interface.h"

namespace dxup {
  HRESULT CreateD3D9(IDirect3D9Ex** d3d9) {
    InitReturnPtr(d3d9);

    if (d3d9 == nullptr)
      return E_INVALIDARG;

    Com<IDXGIFactory1> dxgiFactory;
    HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&dxgiFactory);

    if (FAILED(result))
        return result;

    *d3d9 = ref(new Direct3D9Ex(dxgiFactory.ptr()));

    return D3D_OK;
  }
}

IDirect3D9* STDMETHODCALLTYPE Direct3DCreate9(UINT nSDKVersion) {
  IDirect3D9Ex* pDirect3D = nullptr;
  dxup::CreateD3D9(&pDirect3D);

  return pDirect3D;
}

HRESULT STDMETHODCALLTYPE Direct3DCreate9Ex(UINT nSDKVersion, IDirect3D9Ex** ppDirect3D9Ex) {
  return dxup::CreateD3D9(ppDirect3D9Ex);
}

int STDMETHODCALLTYPE D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName) {
  dxup::log::stub("D3DPERF_BeginEvent");
  return 0;
}

int STDMETHODCALLTYPE D3DPERF_EndEvent(void) {
  dxup::log::stub("D3DPERF_EndEvent");
  return 0;
}

void STDMETHODCALLTYPE D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName) {
  dxup::log::stub("D3DPERF_SetMarker");
}

void STDMETHODCALLTYPE D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName) {
  dxup::log::stub("D3DPERF_SetRegion");
}

BOOL STDMETHODCALLTYPE D3DPERF_QueryRepeatFrame(void) {
  dxup::log::stub("D3DPERF_QueryRepeatFrame");
  return FALSE;
}

void STDMETHODCALLTYPE D3DPERF_SetOptions(DWORD dwOptions) {
  dxup::log::stub("D3DPERF_SetOptions");
}

DWORD STDMETHODCALLTYPE D3DPERF_GetStatus(void) {
  dxup::log::stub("D3DPERF_GetStatus");
  return 0;
}