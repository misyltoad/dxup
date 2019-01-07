#pragma once

#include "dxbc_helpers.h"
#include <d3d9types.h>

namespace dxup {

  namespace dx9asm {

    enum class UsageType {
      None,
      Input,
      Output
    };

    struct DclInfo {
      UsageType type;
      D3DDECLUSAGE usage;
      uint32_t usageIndex;
      bool target = false;
      bool centroid = false;
    };

    struct RegisterMapping {
      uint32_t dx9Type;
      uint32_t dx9Id;
      DXBCOperand dxbcOperand;

      DXBCOperand relativeOperand;
      bool hasRelativeOperand = false;

      uint32_t readMask = 0;
      uint32_t writeMask = 0;

      DclInfo dclInfo;
    };

  }

}