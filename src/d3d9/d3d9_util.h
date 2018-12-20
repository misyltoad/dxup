#pragma once

#include "d3d9_includes.h"
#include "../util/log.h"
#include "../util/shared_conversions.h"

namespace dxup {

  namespace convert {
    DXGI_FORMAT format(D3DFORMAT Format);
    D3DFORMAT format(DXGI_FORMAT Format);

    DXGI_MODE_SCANLINE_ORDER scanlineOrdering(D3DSCANLINEORDERING ScanlineOrdering);
    D3DSCANLINEORDERING scanlineOrdering(DXGI_MODE_SCANLINE_ORDER ScanlineOrdering);

    UINT cpuFlags(D3DPOOL pool, UINT usage);
    D3D11_USAGE usage(D3DPOOL pool, UINT usage);

    UINT primitiveData(D3DPRIMITIVETYPE type, UINT count, D3D_PRIMITIVE_TOPOLOGY& topology);

    DXGI_FORMAT declType(D3DDECLTYPE type);

    void color(D3DCOLOR color, FLOAT* d3d11Color);
  }

  template <typename T>
  void makeStaging(T& desc, UINT d3d9Usage) {
    desc.CPUAccessFlags = d3d9Usage & D3DUSAGE_WRITEONLY ? D3D11_CPU_ACCESS_WRITE : D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
  }

}