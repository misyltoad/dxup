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

    void DXBCOperand::doPass(uint32_t* instructionSize, std::vector<uint32_t>* code) {
      if (code != nullptr) {
        uint32_t header = ENCODE_D3D10_SB_OPERAND_TYPE(m_registerType) |
          ENCODE_D3D10_SB_OPERAND_INDEX_DIMENSION(m_dimension) |
          ENCODE_D3D10_SB_OPERAND_EXTENDED(m_hasExtension) |
          ENCODE_D3D10_SB_OPERAND_NUM_COMPONENTS(m_components == 4 ? D3D10_SB_OPERAND_4_COMPONENT : m_components == 1 ? D3D10_SB_OPERAND_1_COMPONENT : D3D10_SB_OPERAND_0_COMPONENT) |
          m_swizzleOrWritemask;

        for (uint32_t i = 0; i < m_representations; i++)
          header |= ENCODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(i, m_representation[i]);

        code->push_back(header);
      }
      
      if (instructionSize != nullptr)
        (*instructionSize)++;

      if (m_hasExtension) {
        if (code != nullptr)
          code->push_back(ENCODE_D3D10_SB_EXTENDED_OPERAND_MODIFIER(m_extension));

        if (instructionSize != nullptr)
          (*instructionSize)++;
      }

      for (uint32_t i = 0; i < m_dataCount; i++) {
        if (code != nullptr)
          code->push_back(m_data[i]);

        if (instructionSize != nullptr)
          (*instructionSize)++;
      }
    }

    uint32_t DXBCOperation::genOpToken(uint32_t instructionSize) {
      return ENCODE_D3D10_SB_OPCODE_TYPE(m_opcode) |
             ENCODE_D3D10_SB_INSTRUCTION_SATURATE(m_saturate) |
             ENCODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(instructionSize);
    }

    void DXBCOperation::push(std::vector<uint32_t>& code) {
      uint32_t instructionSize = 1; // Opcode Token

      if (m_lengthOverride == -1) {
        // Pass 1 - Size
        for (size_t i = 0; i < m_operands.size(); i++)
          m_operands[i].addInstructionSize(instructionSize);
      }
      else
        instructionSize = m_lengthOverride;

      // Pass 2 - Push
      code.push_back(genOpToken(instructionSize));
      for (size_t i = 0; i < m_operands.size(); i++)
        m_operands[i].push(code);
    }

    void DXBCOperation::push(ShaderCodeTranslator& translator) {
      push(translator.getCode());
    }
  }

}