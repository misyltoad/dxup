#pragma once

#include "dx9asm_meta.h"
#include "../util/fixed_buffer.h"

namespace dxapex {

  namespace dx9asm {

    class DX9Operand;
    class ShaderCodeTranslator;
    class DXBCOperand;

    uint32_t byteCodeLength(const uint32_t* dx9asm);

    uint32_t opcode(uint32_t token);

    void convertRegTypeAndNum(ShaderCodeTranslator& state, DXBCOperand& dstOperand, const DX9Operand& operand, uint32_t regOffset);

  }

}

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))