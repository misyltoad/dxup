#pragma once

#include "dx9asm_meta.h"
#include "../util/fixed_buffer.h"
#include "dx9asm_operand.h"
#include <vector>

namespace dxapex {

  namespace dx9asm {

    class ShaderCodeTranslator;
    class DX9Operation;

    namespace implicitflag {
      const uint32_t negate = 1;
      const uint32_t abs = 2;
    }

    struct DX9ToDXBCImplicitConversionInfo {
      uint32_t dxbcOpcode = 0;
      uint32_t implicitFlags = 0;
    };

    typedef bool(ShaderCodeTranslator::*UniqueFunction)(DX9Operation&);

    struct DX9OperationInfo {
      const char* name;
      uint32_t dx9opcode;
      uint32_t matrixColumns;
      std::vector<OperandType> args;

      DX9ToDXBCImplicitConversionInfo implicitInfo;
      UniqueFunction uniqueFunction = nullptr;
    };

    const DX9OperationInfo* lookupOperationInfo(uint32_t token);

    class DX9Operation {
    public:
      DX9Operation(ShaderCodeTranslator& translator, uint32_t token)
        : m_dx9token{ token } {
        m_info = lookupOperationInfo(token);

        if (m_info) {
          m_operands.reserve(m_info->args.size());

          readOperands(translator);
        }
      }

      inline const DX9ToDXBCImplicitConversionInfo& getImplicitInfo() const {
        return m_info->implicitInfo;
      }

      inline bool isValid() const {
        return m_info != nullptr;
      }

      inline uint32_t getToken() const {
        return m_dx9token;
      }

      inline uint32_t getOpcode() const {
        return opcode(m_dx9token);
      }

      inline uint32_t getMatrixColumns() const {
        return m_info->matrixColumns;
      }

      inline const std::vector<OperandType>& getArgs() const {
        return m_info->args;
      }

      inline const char* getName() const {
        return m_info->name;
      }

      inline bool isUnique() const {
        return getUniqueFunction() != nullptr;
      }

      inline uint32_t getCommentCount() const {
        return (getToken() & D3DSI_COMMENTSIZE_MASK) >> 16;
      }

      inline size_t operandCount() const {
        return m_operands.size();
      }

      inline const DX9Operand* getOperandByIndex(size_t i) const {
        return &m_operands[i];
      }

      inline UniqueFunction getUniqueFunction() const {
        return m_info->uniqueFunction;
      }

      inline bool saturate() const {
        const DX9Operand* operand = getOperandByType(optype::Dst);
        if (operand != nullptr)
          return operand->getSaturate();

        return false;
      }

      inline const DX9Operand* getOperandByType(OperandType type) const {
        for (size_t i = 0; i < m_operands.size(); i++) {
          const DX9Operand& operand = m_operands[i];
          if (operand.getType() == type)
            return &operand;
        }
        return nullptr;
      }

    private:

      void readOperands(ShaderCodeTranslator& translator);

      std::vector<DX9Operand> m_operands;
      const DX9OperationInfo* m_info;
      uint32_t m_dx9token;
    };
  }

}