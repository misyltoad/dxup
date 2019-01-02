#pragma once

#include "d3d9_base.h"

#define D3DFMT_INTZ ((D3DFORMAT)(MAKEFOURCC('I','N','T','Z')))
#define D3DFMT_RAWZ ((D3DFORMAT)(MAKEFOURCC('R','A','W','Z')))
#define D3DFMT_DF24 ((D3DFORMAT)(MAKEFOURCC('D','F','2','4')))
#define D3DFMT_DF16 ((D3DFORMAT)(MAKEFOURCC('D','F','1','6')))
#define D3DFMT_INST ((D3DFORMAT)(MAKEFOURCC('I','N','S','T')))
#define D3DFMT_NULL ((D3DFORMAT)(MAKEFOURCC('N','U','L','L')))

namespace dxup {

  UINT bitsPerPixel(DXGI_FORMAT fmt);
  UINT alignment(DXGI_FORMAT fmt);

  uint32_t alignRectForFormat(bool down, DXGI_FORMAT format, uint32_t measure);

}