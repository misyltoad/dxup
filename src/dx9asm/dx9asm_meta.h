#pragma once

#include "../util/windows_includes.h"
#include <d3d9.h>
#include <d3d11.h>
#include "../extern/microsoft/d3d11TokenizedProgramFormat.hpp"
#include <stdint.h>

namespace dxapex {

  namespace dx9asm {

    enum class ShaderType {
      Vertex,
      Pixel
    };

    const uint32_t CTAB_CONSTANT = MAKEFOURCC('C', 'T', 'A', 'B');

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