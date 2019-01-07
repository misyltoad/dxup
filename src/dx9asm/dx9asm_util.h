#pragma once

#include "dx9asm_meta.h"
#include "../util/fixed_buffer.h"

namespace dxup {

  namespace dx9asm {

    class DX9Operand;
    class ShaderCodeTranslator;
    class DXBCOperand;

    uint32_t byteCodeLength(const uint32_t* dx9asm);

    uint32_t opcode(uint32_t token);
  }

}