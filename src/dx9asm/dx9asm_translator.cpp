#include "dx9asm_translator.h"
#include "dx9asm_operations.h"
#include "../util/misc_helpers.h"
#include "../util/log.h"
#include "../util/config.h"
#include "dx9asm_operation_helpers.h"
#include "dxbc_helpers.h"
#include <vector>
#include <functional>

namespace dxapex {

  namespace dx9asm {

    bool ShaderCodeTranslator::handleComment(DX9Operation& operation) {
      uint32_t commentTokenCount = operation.getCommentCount();

      uint32_t fourcc = nextToken();
      if (fourcc == CTAB_CONSTANT) {
        m_ctab = (CTHeader*)(m_head);
        uint32_t tableSize = (commentTokenCount - 1) * sizeof(uint32_t);

        if (tableSize < sizeof(CTHeader) || m_ctab->size != sizeof(CTHeader)) {
          log::fail("CTAB invalid!");
          return false; // fatal
        }
      }

      skipTokens(commentTokenCount - 1);
      return true;
    }

    bool ShaderCodeTranslator::handleDef(DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* vec4 = operation.getOperandByType(optype::Vec4);

      RegisterMapping mapping;
      mapping.dx9Id = dst->getRegNumber();
      mapping.dx9Type = dst->getRegType();
      mapping.dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_IMMEDIATE32);
      mapping.dxbcOperand.setExtension(false);
      mapping.dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_0D);
      uint32_t data[4];
      vec4->getValues(data);
      mapping.dxbcOperand.setData(data, 4);
      for (uint32_t i = 0; i < 4; i++)
        mapping.dxbcOperand.setRepresentation(i, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);

      mapping.dxbcOperand.setSwizzleOrWritemask(noSwizzle);

      addRegisterMapping(false, mapping);

      return true;
    }

    bool ShaderCodeTranslator::handleUniqueOperation(DX9Operation& operation) {
      UniqueFunction function = operation.getUniqueFunction();
      if (function == nullptr) {
        log::fail("Unimplemented operation %s encountered.", operation.getName());
        return true; // nonfatal
      }

      return std::invoke(function, this, operation);
    }

    bool ShaderCodeTranslator::handleStandardOperation(DX9Operation& operation) {
      if (!config::getBool(config::EmitNop) && operation.getImplicitInfo().dxbcOpcode == D3D10_SB_OPCODE_NOP)
        return true;

      for (uint32_t col = 0; col < operation.getMatrixColumns(); col++) {
        DXBCOperation dxbcOperation{operation};

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

    bool ShaderCodeTranslator::handleOperation(uint32_t token) {
      DX9Operation operation{ *this, token };
      if (!operation.isValid()) {
        log::fail("Unknown operation encountered!");
        return false;
      }

      if (config::getBool(config::ShaderSpew))
        log::msg("Translating operation %s.", operation.getName());

      if (operation.isUnique())
        return handleUniqueOperation(operation);
      else
        return handleStandardOperation(operation);

      return true;
    }

    bool ShaderCodeTranslator::translate() {
      nextToken(); // Skip header.

      uint32_t token = nextToken();
      while (token != D3DPS_END()) {
        if (!handleOperation(token))
          return false;
        token = nextToken();
      }

      return true;
    }
  
    void toDXBC(const uint32_t* dx9asm, ITranslatedShaderDXBC** dxbc) {
      InitReturnPtr(dxbc);

      ShaderCodeTranslator translator{dx9asm};
      if (!translator.translate()) {
        log::fail("Failed to translate shader fatally!");
        return;
      }
    }

  }

}