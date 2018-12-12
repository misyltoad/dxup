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

      DXBCOperand(uint32_t regType, uint32_t dimension) {
        m_registerType = regType;
        m_dimension = dimension;
      }

      inline uint32_t getRegisterType() const {
        return m_registerType;
      }

      inline DXBCOperand& setupLiteral(uint32_t components) {
        m_components = components;
        for (uint32_t i = 0; i < 4; i++)
          setRepresentation(i, 0);
        return *this;
      }

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
      inline DXBCOperand& setData(const uint32_t* data, uint32_t count) {
        if (count > 4) {
          log::fail("Setting more data than buffer for operand allows!");
          count = 4;
        }
        std::memcpy(m_data, data, count * sizeof(uint32_t));
        m_dataCount = count;

        return *this;
      }

      // I'm not a fan of this & here. ~ Josh
      inline uint32_t& getRegNumber() {
        if (m_registerType == D3D10_SB_OPERAND_TYPE_IMMEDIATE32) {
          log::fail("Attempted to get the register of a literal!");
          return m_dummy;
        }

        return m_data[m_dataCount - 1];
      }

      inline bool isLiteral() const {
        return m_registerType == D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
      }

      inline uint32_t getRegNumber() const {
        if (isLiteral()) {
          log::fail("Attempted to get the register of a literal!");
          return m_dummy;
        }

        return m_data[m_dataCount - 1];
      }

      inline void addInstructionSize(uint32_t& instructionSize) {
        doPass(&instructionSize, nullptr);
      }

      inline void push(std::vector<uint32_t>& code) {
        doPass(nullptr, &code);
      }

    protected:

      void doPass(uint32_t* instructionSize, std::vector<uint32_t>* code);

      // TODO: We could remove these next 3 later on and do encoding straight away. ~ Josh
      uint32_t m_registerType = 0;
      uint32_t m_dimension = 0;
      uint32_t m_representation[4] = { 0 };
      uint32_t m_representations = 0;
      uint32_t m_swizzleOrWritemask = 0;
      uint32_t m_components = 4;
      uint32_t m_extension = 0;
      uint32_t m_data[4] = { 0 };
      uint32_t m_dataCount = 0;
      uint32_t m_dummy = 0;
      bool m_hasExtension = false;
    };

    class DXBCOperation {
    public:
      DXBCOperation(uint32_t opcode, bool saturate, uint32_t lengthOverride = UINT32_MAX, uint32_t lengthOffset = 0, uint32_t interpolationMode = UINT32_MAX)
        : m_opcode{ opcode }, m_saturate{ saturate }, m_lengthOverride{ lengthOverride }, m_lengthOffset{ lengthOffset }, m_interpolationMode{ interpolationMode } {

        if (m_lengthOverride == UINT32_MAX)
          m_operands.reserve(4);
      }

      DXBCOperation(const DX9Operation& operation)
        : DXBCOperation{ operation.getImplicitInfo().dxbcOpcode, operation.saturate() } {}

      inline void setOpcode(uint32_t opcode) {
        m_opcode = opcode;
      }
      inline void setSaturate(bool saturate) {
        m_saturate = saturate;
      }
      inline DXBCOperation& appendOperand(const DXBCOperand& operand) {
        m_operands.push_back(operand);
        return *this;
      }

      void push(std::vector<uint32_t>& code);
      void push(ShaderCodeTranslator& translator);
    private:
      uint32_t genOpToken(uint32_t instructionSize);

      uint32_t m_opcode = 0;
      uint32_t m_lengthOverride = UINT32_MAX;
      uint32_t m_interpolationMode = UINT32_MAX;
      uint32_t m_lengthOffset = 0;
      bool m_saturate = false;
      std::vector<DXBCOperand> m_operands;
    };

  }

}