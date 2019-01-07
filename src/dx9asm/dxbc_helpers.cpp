#include "dxbc_helpers.h"
#include "dx9asm_modifiers.h"
#include "dx9asm_translator.h"
#include "dx9asm_register_map.h"
#include "dx9asm_util.h"
#include "../extern/microsoft/d3d11TokenizedProgramFormat.hpp"

namespace dxup {

  namespace dx9asm {

    DXBCOperand::DXBCOperand(ShaderCodeTranslator& state, const DX9Operation& operation, const DX9Operand& operand, uint32_t regOffset) {
      RegisterMapping* mapping = state.getRegisterMap().lookupOrCreateRegisterMapping(state, operand, regOffset);
      std::memcpy(this, &mapping->dxbcOperand, sizeof(DXBCOperand));

      if (operand.isIndirect()) {
        const uint32_t constantBufferIndex = 0;
        uint32_t dataWithDummyId[2] = { constantBufferIndex, operand.getRegNumber() };
        setData(dataWithDummyId, 2);

        setRegisterType(D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER);
        setDimension(D3D10_SB_OPERAND_INDEX_2D);

        setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
        setRepresentation(1, D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE);

        uint32_t dx9Swizzle = operand.getRelativeAddrSwizzle(state.getMajorVersion());
        const RegisterMapping* address = state.getRegisterMap().lookupOrCreateRegisterMapping(state, D3DSPR_ADDR, operand.getRelativeAddrIndex(state.getMajorVersion()), calcReadMask(dx9Swizzle, 1), 0, false );
        m_relativeIndex = address->dxbcOperand.getRegNumber();

        if (dx9Swizzle & D3DVS_X_Y)
          m_relativeSelect = ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(D3D10_SB_4_COMPONENT_Y);
        else if (dx9Swizzle & D3DVS_X_Z)
          m_relativeSelect = ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(D3D10_SB_4_COMPONENT_Z);
        else if (dx9Swizzle & D3DVS_X_W)
          m_relativeSelect = ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(D3D10_SB_4_COMPONENT_W);
        else
          m_relativeSelect = ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(D3D10_SB_4_COMPONENT_X);

        m_relativeSelect = ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE) | m_relativeSelect;
      } else if (isLiteral()) {
        if (operand.isSrc())
          setSwizzleOrWritemask(noSwizzle);
        else
          setSwizzleOrWritemask(writeAll);

        uint32_t originalData[4];
        std::memcpy(originalData, m_data, 4 * sizeof(uint32_t));

        uint32_t dx9Swizzle = operand.getSwizzleData() >> D3DVS_SWIZZLE_SHIFT;

        for (uint32_t i = 0; i < 4; i++) {
          uint32_t shift = i * 2;
          uint32_t mask = 0b0011u << shift;
          uint32_t swizzleIndex = (dx9Swizzle & mask) >> shift;
          m_data[i] = originalData[swizzleIndex];
        }

        return;
      }

      calculateDXBCSwizzleAndWriteMask(*this, operand);
      calculateDXBCModifiers(*this, operation, operand);
    }

    void DXBCOperand::doPass(uint32_t* instructionSize, std::vector<uint32_t>* code) {
      if (code != nullptr) {
        uint32_t header = ENCODE_D3D10_SB_OPERAND_TYPE(m_registerType) |
          ENCODE_D3D10_SB_OPERAND_INDEX_DIMENSION(m_dimension) |
          ENCODE_D3D10_SB_OPERAND_EXTENDED(m_modifier != 0) |
          ENCODE_D3D10_SB_OPERAND_NUM_COMPONENTS(m_components == 4 ? D3D10_SB_OPERAND_4_COMPONENT : m_components == 1 ? D3D10_SB_OPERAND_1_COMPONENT : D3D10_SB_OPERAND_0_COMPONENT) |
          m_swizzleOrWritemask;

        for (uint32_t i = 0; i < m_representations; i++)
          header |= ENCODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(i, m_representation[i]);

        code->push_back(header);
      }
      
      if (instructionSize != nullptr)
        (*instructionSize)++;

      if (m_modifier != 0) {
        if (code != nullptr)
          code->push_back(ENCODE_D3D10_SB_EXTENDED_OPERAND_MODIFIER(m_modifier));

        if (instructionSize != nullptr)
          (*instructionSize)++;
      }

      for (uint32_t i = 0; i < m_dataCount; i++) {
        if (code != nullptr)
          code->push_back(m_data[i]);

        if (instructionSize != nullptr)
          (*instructionSize)++;
      }

      //

      if (m_relativeIndex != UINT32_MAX) {
        if (code != nullptr) {
          uint32_t header = ENCODE_D3D10_SB_OPERAND_TYPE(D3D10_SB_OPERAND_TYPE_TEMP) |
            ENCODE_D3D10_SB_OPERAND_INDEX_DIMENSION(D3D10_SB_OPERAND_INDEX_1D) |
            ENCODE_D3D10_SB_OPERAND_NUM_COMPONENTS(D3D10_SB_OPERAND_4_COMPONENT) |
            ENCODE_D3D10_SB_OPERAND_INDEX_REPRESENTATION(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32) |
            m_relativeSelect;

          code->push_back(header);
        }

        if (instructionSize != nullptr)
          (*instructionSize)++;

        if (code != nullptr)
          code->push_back(m_relativeIndex);

        if (instructionSize != nullptr)
          (*instructionSize)++;
      }
    }

    void DXBCOperation::pushOpTokens(std::vector<uint32_t>& code, uint32_t operandSize) {
      uint32_t instructionSize = operandSize;
      uint32_t operationExtension[2] = { 0, 0 };

      if (m_lengthOverride == UINT32_MAX)
        instructionSize += m_sampler ? 3 : 1;
      else
        instructionSize = m_lengthOverride;

      instructionSize += m_lengthOffset;
        
      uint32_t opToken = ENCODE_D3D10_SB_OPCODE_TYPE(m_opcode) |
                         ENCODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(instructionSize) |
                         ENCODE_D3D10_SB_OPCODE_EXTENDED(m_sampler);

      if (m_extra == 0) {
        if (m_flags != UINT32_MAX)
          opToken |= ENCODE_D3D10_SB_GLOBAL_FLAGS(m_flags);
        else
          opToken |= ENCODE_D3D10_SB_INSTRUCTION_SATURATE(m_saturate);
      }
      else
        opToken |= m_extra;

      if (m_interpolationMode != UINT32_MAX)
        opToken |= ENCODE_D3D10_SB_INPUT_INTERPOLATION_MODE(m_interpolationMode);

      if (m_sampler) {
        operationExtension[0] |= ENCODE_D3D10_SB_EXTENDED_OPCODE_TYPE(D3D11_SB_EXTENDED_OPCODE_RESOURCE_DIM);
        operationExtension[0] |= ENCODE_D3D11_SB_EXTENDED_RESOURCE_DIMENSION(D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D);
        operationExtension[0] |= ENCODE_D3D10_SB_OPCODE_EXTENDED(true);

        operationExtension[1] |= ENCODE_D3D10_SB_EXTENDED_OPCODE_TYPE(D3D11_SB_EXTENDED_OPCODE_RESOURCE_RETURN_TYPE);
        operationExtension[1] |= ENCODE_D3D11_SB_EXTENDED_RESOURCE_RETURN_TYPE(D3D10_SB_RETURN_TYPE_FLOAT, 0);
        operationExtension[1] |= ENCODE_D3D11_SB_EXTENDED_RESOURCE_RETURN_TYPE(D3D10_SB_RETURN_TYPE_FLOAT, 1);
        operationExtension[1] |= ENCODE_D3D11_SB_EXTENDED_RESOURCE_RETURN_TYPE(D3D10_SB_RETURN_TYPE_FLOAT, 2);
        operationExtension[1] |= ENCODE_D3D11_SB_EXTENDED_RESOURCE_RETURN_TYPE(D3D10_SB_RETURN_TYPE_FLOAT, 3);
      }           

      code.push_back(opToken);

      if (m_sampler) {
        code.push_back(operationExtension[0]);
        code.push_back(operationExtension[1]);
      }
    }

    void DXBCOperation::push(std::vector<uint32_t>& code) {
      uint32_t instructionSize = 0;

      // Pass 1 - Size
      for (size_t i = 0; i < m_operands.size(); i++)
        m_operands.get(i).addInstructionSize(instructionSize);

      // Pass 2 - Push
      pushOpTokens(code, instructionSize);
      for (size_t i = 0; i < m_operands.size(); i++)
        m_operands.get(i).push(code);
    }

    void DXBCOperation::push(ShaderCodeTranslator& translator) {
      push(translator.getCode());
    }
  }

}