#include "dx9asm_util.h"
#include "dx9asm_operand.h"
#include "dx9asm_operations.h"
#include "dx9asm_translator.h"
#include "dxbc_helpers.h"

namespace dxup {

  namespace dx9asm {

    uint32_t byteCodeLength(const uint32_t* dx9asm) {
      if (dx9asm == nullptr)
        return 0;

      const uint32_t* start = dx9asm;
      while (*dx9asm != D3DPS_END())
        dx9asm++;

      return (uint32_t)( (size_t)dx9asm - (size_t)start );
    }

    uint32_t opcode(uint32_t token) {
      return token & D3DSI_OPCODE_MASK;
    }

  }

}