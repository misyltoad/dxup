#include "dxbc_helpers.h"
#include "dx9asm_modifiers.h"
#include "dx9asm_translator.h"
#include "dx9asm_util.h"
#include "../extern/microsoft/d3d11TokenizedProgramFormat.hpp"

namespace dxapex {

  namespace dx9asm {

    DXBCOperand::DXBCOperand(ShaderCodeTranslator& state, const DX9Operation& operation, const DX9Operand& operand, uint32_t regOffset) {
      RegisterMapping* mapping = state.lookupOrCreateRegisterMapping(operand, regOffset);
      std::memcpy(this, &mapping->dxbcOperand, sizeof(DXBCOperand));

      calculateDXBCSwizzleAndWriteMask(*this, operand);
      calculateDXBCModifiers(*this, operation, operand);
    }

    void DXBCOperand::doPass(uint32_t* instructionSize, ShaderCodeTranslator* translator) {
      uint32_t header = ENCODE_D3D10_SB_OPERAND_TYPE(m_registerType) |
                        ENCODE_D3D10_SB_OPERAND_INDEX_DIMENSION(m_dimension) |
                        ENCODE_D3D10_SB_OPERAND_EXTENDED(m_hasExtension) |
                        m_swizzleOrWritemask;

      for (uint32_t i = 0; i < m_representations; i++)
        header |= ENCODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(i, m_representation[i]);

      if (translator != nullptr)
        translator->push(header);
      
      if (instructionSize != nullptr)
        (*instructionSize)++;

      if (m_hasExtension) {
        if (translator != nullptr)
          translator->push(ENCODE_D3D10_SB_EXTENDED_OPERAND_TYPE(D3D10_SB_EXTENDED_OPERAND_MODIFIER) | ENCODE_D3D10_SB_EXTENDED_OPERAND_MODIFIER(m_extension));

        if (instructionSize != nullptr)
          (*instructionSize)++;
      }

      for (uint32_t i = 0; i < m_dataCount; i++) {
        if (translator != nullptr)
          translator->push(m_data[i]);

        if (instructionSize != nullptr)
          (*instructionSize)++;
      }
    }

    uint32_t DXBCOperation::genOpToken(uint32_t instructionSize) {
      return ENCODE_D3D10_SB_OPCODE_TYPE(m_opcode) |
             ENCODE_D3D10_SB_INSTRUCTION_SATURATE(m_saturate) |
             ENCODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(instructionSize);
    }

    void DXBCOperation::push(ShaderCodeTranslator& translator) {
      uint32_t instructionSize = 1; // Opcode Token

      // Pass 1 - Size
      for (size_t i = 0; i < m_operands.size(); i++)
       m_operands[i].addInstructionSize(instructionSize); 

      // Pass 2 - Push
      translator.push(genOpToken(instructionSize));
      for (size_t i = 0; i < m_operands.size(); i++)
       m_operands[i].push(translator);
    }
  }

}