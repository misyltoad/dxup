#pragma once

#include <stdint.h>
#include <string.h>
#include <vector>
#include "../util/fixed_buffer.h"
#include "dx9asm_operations.h"
#include <algorithm>

namespace dxup {

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

      DXBCOperand(uint32_t literal0, uint32_t literal1, uint32_t literal2, uint32_t literal3) {
        uint32_t data[4] = { literal0, literal1, literal2, literal3 };
        setupLiteral(4);
        setData(data, 4);
      }

      inline uint32_t getRegisterType() const {
        return m_registerType;
      }

      inline DXBCOperand& setComponents(uint32_t components) {
        m_components = components;
        return *this;
      }

      inline DXBCOperand& setupLiteral(uint32_t components) {
        m_components = components;
        stripModifier();
        setRegisterType(D3D10_SB_OPERAND_TYPE_IMMEDIATE32);
        setDimension(D3D10_SB_OPERAND_INDEX_0D);
        for (uint32_t i = 0; i < 4; i++)
          setRepresentation(i, 0);
        return *this;
      }

      inline DXBCOperand& setRegisterType(uint32_t regType) {
        m_registerType = regType;
        return *this;
      }
      inline DXBCOperand& setDimension(uint32_t dimension) {
        m_dimension = dimension;
        return *this;
      }
      inline DXBCOperand& setRepresentation(uint32_t index, uint32_t representation) {
        m_representation[index] = representation;
        m_representations = std::max(m_representations, index + 1);
        return *this;
      }
      inline DXBCOperand& setSwizzleOrWritemask(uint32_t swizzlewrite) {
        m_swizzleOrWritemask = swizzlewrite;
        return *this;
      }
      inline uint32_t getSwizzleOrWritemask() const {
        return m_swizzleOrWritemask;
      }
      inline uint32_t getModifier() {
        return m_modifier;
      }
      inline DXBCOperand& setModifier(uint32_t modifier) {
        m_modifier = modifier;
        return *this;
      }
      inline DXBCOperand& stripModifier() {
        m_modifier = 0;
        return *this;
      }
      inline DXBCOperand& setData(const uint32_t* data, uint32_t count) {
        if (count > 4) {
          log::fail("Setting more data than buffer for operand allows!");
          count = 4;
        }

        if (data)
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

      inline DXBCOperand& addInstructionSize(uint32_t& instructionSize) {
        doPass(&instructionSize, nullptr);
        return *this;
      }

      inline DXBCOperand& push(std::vector<uint32_t>& code) {
        doPass(nullptr, &code);
        return *this;
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
      uint32_t m_modifier = 0;
      uint32_t m_data[4] = { 0 };
      uint32_t m_dataCount = 0;
      uint32_t m_dummy = 0;
      uint32_t m_relativeIndex = UINT32_MAX;
      uint32_t m_relativeSelect = UINT32_MAX;
      bool m_hasExtension = false;
    };

    class DXBCOperation {
    public:
      DXBCOperation(uint32_t opcode, bool saturate, uint32_t lengthOverride = UINT32_MAX, uint32_t lengthOffset = 0, uint32_t interpolationMode = UINT32_MAX, uint32_t flags = UINT32_MAX )
        : m_opcode{ opcode }
        , m_saturate{ saturate }
        , m_lengthOverride{ lengthOverride }
        , m_lengthOffset{ lengthOffset }
        , m_interpolationMode{ interpolationMode }
        , m_flags{ flags } {
      }

      DXBCOperation(const DX9Operation& operation)
        : DXBCOperation{ operation.getImplicitInfo().dxbcOpcode, operation.saturate() } {}

      inline DXBCOperation& setSampler(bool sampler) {
        m_sampler = sampler;
        return *this;
      }

      inline DXBCOperation& setOpcode(uint32_t opcode) {
        m_opcode = opcode;
        return *this;
      }
      inline DXBCOperation& setSaturate(bool saturate) {
        m_saturate = saturate;
        return *this;
      }
      inline DXBCOperation& appendOperand(const DXBCOperand& operand) {
        m_operands.push_back(operand);
        return *this;
      }

      inline DXBCOperation& setExtra(uint32_t extra) {
        m_extra = extra;
        return *this;
      }

      void push(std::vector<uint32_t>& code);
      void push(ShaderCodeTranslator& translator);
    private:
      void pushOpTokens(std::vector<uint32_t>& code, uint32_t operandSize);

      uint32_t m_opcode = 0;
      uint32_t m_lengthOverride = UINT32_MAX;
      uint32_t m_interpolationMode = UINT32_MAX;
      uint32_t m_flags = UINT32_MAX;
      uint32_t m_lengthOffset = 0;
      bool m_saturate = false;
      bool m_sampler = false;
      uint32_t m_extra = 0;
      FixedBuffer<8, DXBCOperand> m_operands;
    };

  }

}