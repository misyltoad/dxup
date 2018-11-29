#pragma once

#include "dx9asm_meta.h"

namespace dxapex {

  namespace dx9asm {

    class DXBCOperand;
    class DX9Operation;
    class DX9Operand;

    void calculateDXBCModifiers(DXBCOperand& dstOperand, const DX9Operation& operation, const DX9Operand& operand);

    void calculateDXBCSwizzleAndWriteMask(DXBCOperand& dstOperand, const DX9Operand& operand);

  }

}