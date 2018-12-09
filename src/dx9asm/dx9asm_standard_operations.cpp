#include "dx9asm_translator.h"
#include "../util/config.h"

namespace dxapex {

  namespace dx9asm {

    bool ShaderCodeTranslator::handleStandardOperation(DX9Operation& operation) {
      if (!config::getBool(config::EmitNop) && operation.getImplicitInfo().dxbcOpcode == D3D10_SB_OPCODE_NOP)
        return true;

      for (uint32_t col = 0; col < operation.getMatrixColumns(); col++) {
        DXBCOperation dxbcOperation{ operation };

        for (uint32_t i = 0; i < operation.operandCount(); i++) {
          const DX9Operand* operand = operation.getOperandByIndex(i);

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