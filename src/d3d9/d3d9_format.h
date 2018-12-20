#pragma once

#include "d3d9_base.h"

namespace dxup {

  UINT alignPitch(UINT pitch);


  struct DXGIFormatSize {
    UINT pixelBytes;
    UINT blockWidth;
    UINT blockHeight;
  };

  const DXGIFormatSize &getDXGIFormatSizeInfo(DXGI_FORMAT format);
  UINT calculatePitch(DXGI_FORMAT format, UINT width);

}