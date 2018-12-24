#pragma once

#include "d3d9_base.h"

namespace dxup {

  struct DXGIFormatSize {
    UINT pixelBytes;
    UINT blockWidth;
    UINT blockHeight;
  };

  const DXGIFormatSize &getDXGIFormatSizeInfo(DXGI_FORMAT format);

  uint32_t alignWidthForFormat(bool down, DXGI_FORMAT format, uint32_t width);
  uint32_t alignHeightForFormat(bool down, DXGI_FORMAT format, uint32_t height);

}