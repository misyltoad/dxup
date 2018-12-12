#pragma once

#include "dxbc_helpers.h"
#include <d3d9types.h>

namespace dxapex {

  namespace dx9asm {

    enum class UsageType {
      None,
      Input,
      Output
    };

    struct RegisterMapping {
      uint32_t dx9Type;
      uint32_t dx9Id;
      DXBCOperand dxbcOperand;

      struct DclInfo {
        UsageType type;
        D3DDECLUSAGE usage;
        uint32_t usageIndex;
        bool target = false;
        bool centroid = false;
      } dclInfo;
    };

  }

}