#pragma once

#include <stdint.h>
#include <string.h>
#include <vector>
#include "dx9asm_operations.h"

namespace dxapex {

  namespace dx9asm {

    class ShaderCodeTranslator;

    class DXBCOperand {
    public:

      DXBCOperand() {}

      DXBCOperand(ShaderCodeTranslator& state, const DX9Operation& operation, const DX9Operand& operand, uint32_t regOffset);

      inline void setRegisterType(uint32_t regType) {
        m_registerType = regType;
      }
      inline void setDimension(uint32_t dimension) {
        m_dimension = dimension;
      }
      inline void setRepresentation(uint32_t index, uint32_t representation) {
        m_representation[index] = representation;
        m_representations = max(m_representations, index + 1);
      }
      inline void setSwizzleOrWritemask(uint32_t swizzlewrite) {
        m_swizzleOrWritemask = swizzlewrite;
      }
      inline void setExtension(uint32_t extension) {
        m_hasExtension = true;
        m_extension = extension;
      }
      inline void stripExtension() {
        m_hasExtension = false;
      }
      inline void setData(const uint32_t* data, uint32_t count) {
        if (count > 4) {
          log::fail("Setting more data than buffer for operand allows!");
          count = 4;
        }
        std::memcpy(m_data, data, count * sizeof(uint32_t));
        m_dataCount = count;
      }

      inline void addInstructionSize(uint32_t& instructionSize) {
        doPass(&instructionSize, nullptr);
      }

      inline void push(ShaderCodeTranslator& translator) {
        doPass(nullptr, &translator);
      }

    protected:
      friend class ShaderCodeTranslator;
      void doPass(uint32_t* instructionSize, ShaderCodeTranslator* translator);

      // TODO: We could remove these next 3 later on and do encoding straight away. ~ Josh
      uint32_t m_registerType = 0;
      uint32_t m_dimension = 0;
      uint32_t m_representation[4] = { 0 };
      uint32_t m_representations = 0;
      uint32_t m_swizzleOrWritemask = 0;
      uint32_t m_extension = 0;
      uint32_t m_data[4] = { 0 };
      uint32_t m_dataCount = 0;
      bool m_hasExtension = false;
    };

    class DXBCOperation {
    public:
      DXBCOperation(uint32_t opcode, bool saturate)
        : m_opcode{ opcode }, m_saturate{ saturate } { m_operands.reserve(4); }

      DXBCOperation(const DX9Operation& operation)
        : DXBCOperation{ operation.getImplicitInfo().dxbcOpcode, operation.saturate() } {}

      inline void setOpcode(uint32_t opcode) {
        m_opcode = false;
      }
      inline void setSaturate(bool saturate) {
        m_saturate = saturate;
      }
      inline void appendOperand(const DXBCOperand& operand) {
        m_operands.push_back(operand);
      }
      void push(ShaderCodeTranslator& translator);
    private:
      uint32_t genOpToken(uint32_t instructionSize);

      uint32_t m_opcode;
      bool m_saturate;
      std::vector<DXBCOperand> m_operands;
    };

  }

}