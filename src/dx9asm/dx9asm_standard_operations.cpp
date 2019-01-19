#include "dx9asm_translator.h"
#include "../util/config.h"

namespace dxup {

  namespace dx9asm {

    bool ShaderCodeTranslator::handleStandardOperation(DX9Operation& operation) {
      if (!config::getBool(config::EmitNop) && operation.getImplicitInfo().dxbcOpcode == D3D10_SB_OPCODE_NOP)
        return true;

      for (uint32_t col = 0; col < operation.getMatrixColumns(); col++) {
        DXBCOperation dxbcOperation{ operation };

        for (uint32_t i = 0; i < operation.operandCount(); i++) {
          DX9Operand* operand = operation.getOperandByIndex(i);

          if (operand->getType() == optype::Src0)
            operand->setUsedComponents(operation.getImplicitInfo().componentCounts[0]);
          else if (operand->getType() == optype::Src1)
            operand->setUsedComponents(operation.getImplicitInfo().componentCounts[1]);
          else if (operand->getType() == optype::Src2)
            operand->setUsedComponents(operation.getImplicitInfo().componentCounts[2]);
          else if (operand->getType() == optype::Src3)
            operand->setUsedComponents(operation.getImplicitInfo().componentCounts[3]);

          uint32_t regOffset = operand->getType() == optype::Src1 ? col : 0;
          DXBCOperand dbxcOperand{ *this, operation, *operand, regOffset };
          dxbcOperation.appendOperand(dbxcOperand);
        }

        dxbcOperation.push(*this);
      }

      return true;
    }

  }

}