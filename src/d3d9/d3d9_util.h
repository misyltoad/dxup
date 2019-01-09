#pragma once

#include "d3d9_includes.h"
#include "../util/log.h"
#include "../util/shared_conversions.h"

namespace dxup {

  namespace convert {
    DXGI_FORMAT format(D3DFORMAT Format);
    D3DFORMAT format(DXGI_FORMAT Format);

    DXGI_FORMAT makeUntypeless(DXGI_FORMAT format, bool srgb);
    DXGI_FORMAT makeTypeless(DXGI_FORMAT format);

    DXGI_MODE_SCANLINE_ORDER scanlineOrdering(D3DSCANLINEORDERING ScanlineOrdering);
    D3DSCANLINEORDERING scanlineOrdering(DXGI_MODE_SCANLINE_ORDER ScanlineOrdering);

    UINT cpuFlags(D3DPOOL pool, UINT usage);
    D3D11_USAGE usage(D3DPOOL pool, UINT usage);

    UINT primitiveData(D3DPRIMITIVETYPE type, UINT count, D3D_PRIMITIVE_TOPOLOGY& topology);

    DXGI_FORMAT declType(D3DDECLTYPE type);

    void color(D3DCOLOR color, FLOAT* d3d11Color);

    HRESULT mapStageToSampler(DWORD Stage, DWORD* Sampler);
  }

  template <typename T>
  void makeStagingDesc(T& desc, UINT d3d9Usage, D3DFORMAT format) {
    desc.CPUAccessFlags = d3d9Usage & D3DUSAGE_WRITEONLY ? D3D11_CPU_ACCESS_WRITE : D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    if (format == D3DFMT_R8G8B8)
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
  }

  inline bool isRectDegenerate(RECT rect) {
    return rect.top == 0 && rect.right == 0 && rect.left == 0 && rect.bottom == 0;
  }

  template <typename T, typename J>
  T* useAs(J* obj) {
    return reinterpret_cast<T*>(obj);
  }

}