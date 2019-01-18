#pragma once

#include "../util/windows_includes.h"
#include "../d3d9/d3d9_includes.h"
#include "../extern/microsoft/d3d11TokenizedProgramFormat.hpp"
#include <stdint.h>

namespace dxup {

  namespace dx9asm {

    enum ShaderType {
      Vertex = 0,
      Pixel = 1,
      Count
    };

    struct CTHeader
    {
      uint32_t size;
      uint32_t creator;
      uint32_t version;
      uint32_t constants;
      uint32_t constantInfo;
      uint32_t flags;
      uint32_t target;
    };

  }

}