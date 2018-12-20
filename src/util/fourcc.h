#pragma once

#include "windows_includes.h"
#include <stdint.h>
#include <d3d9.h>

namespace dxup {

  constexpr uint32_t fourcc(const char* str) {
    return MAKEFOURCC(str[0], str[1], str[2], str[3]);
  }

}