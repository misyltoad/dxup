#pragma once

#include "d3d9_base.h"

namespace dxup {

  UINT bitsPerPixel(DXGI_FORMAT fmt);
  UINT bpe(DXGI_FORMAT fmt);

  uint32_t alignRectForFormat(bool down, DXGI_FORMAT format, uint32_t measure);

}