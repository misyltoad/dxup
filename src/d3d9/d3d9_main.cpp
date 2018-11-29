#include <stdio.h>
#include "d3d9_interface.h"

namespace dxapex {
  HRESULT CreateD3D9(IDirect3D9Ex** d3d9) {
    InitReturnPtr(d3d9);

    if (d3d9 == nullptr)
      return E_INVALIDARG;

    IDXGIFactory* dxgiFactory = nullptr;
    HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

    if (FAILED(result))
        return result;

    *d3d9 = ref(new Direct3D9Ex(dxgiFactory));

    return D3D_OK;
  }
}

IDirect3D9* STDMETHODCALLTYPE Direct3DCreate9(UINT nSDKVersion) {
  IDirect3D9Ex* pDirect3D = nullptr;
  dxapex::CreateD3D9(&pDirect3D);

  return pDirect3D;
}

HRESULT STDMETHODCALLTYPE Direct3DCreate9Ex(UINT nSDKVersion, IDirect3D9Ex** ppDirect3D9Ex) {
  return dxapex::CreateD3D9(ppDirect3D9Ex);
}

int STDMETHODCALLTYPE D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName) {
  dxapex::log::stub("D3DPERF_BeginEvent");
  return 0;
}

int STDMETHODCALLTYPE D3DPERF_EndEvent(void) {
  dxapex::log::stub("D3DPERF_EndEvent");
  return 0;
}

void STDMETHODCALLTYPE D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName) {
  dxapex::log::stub("D3DPERF_SetMarker");
}

void STDMETHODCALLTYPE D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName) {
  dxapex::log::stub("D3DPERF_SetRegion");
}

BOOL STDMETHODCALLTYPE D3DPERF_QueryRepeatFrame(void) {
  dxapex::log::stub("D3DPERF_QueryRepeatFrame");
  return FALSE;
}

void STDMETHODCALLTYPE D3DPERF_SetOptions(DWORD dwOptions) {
  dxapex::log::stub("D3DPERF_SetOptions");
}

DWORD STDMETHODCALLTYPE D3DPERF_GetStatus(void) {
  dxapex::log::stub("D3DPERF_GetStatus");
  return 0;
}