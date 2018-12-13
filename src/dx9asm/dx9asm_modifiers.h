#pragma once

#include "dx9asm_meta.h"

namespace dxapex {

  namespace dx9asm {

    class DXBCOperand;
    class DX9Operation;
    class DX9Operand;

    const uint32_t noSwizzle = ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE) | D3D10_SB_OPERAND_4_COMPONENT_NOSWIZZLE;
    const uint32_t writeAll = ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE) | ENCODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL);

    uint32_t calcReadMask(const DX9Operand& operand);
    uint32_t calcWriteMask(const DX9Operand& operand);

    void calculateDXBCModifiers(DXBCOperand& dstOperand, const DX9Operation& operation, const DX9Operand& operand);

    void calculateDXBCSwizzleAndWriteMask(DXBCOperand& dstOperand, const DX9Operand& operand);

  }

}