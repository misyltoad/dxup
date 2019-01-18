#include "dx9asm_operand.h"
#include "dx9asm_translator.h"

namespace dxup {

  namespace dx9asm {

    DX9Operand::DX9Operand(const OperandInfo* info, uint32_t token)
      : m_info{ info } {
      m_dx9tokens[0] = token;
    }

    DX9Operand::DX9Operand(ShaderCodeTranslator& translator, const OperandInfo* info, const uint32_t* tokens)
      : m_info{ info } {
      std::memcpy(m_dx9tokens, tokens, sizeof(uint32_t) * info->sizeInTokens);

      if (isIndirect()) {
        translator.markIndirect();

        if (relativeAddressingUsesToken(translator.getMajorVersion()))
          m_dx9tokens[info->sizeInTokens] = translator.nextToken();
      }
    }

    using namespace optype;
    std::vector<OperandInfo> operandInfos = {
      {Dst, 1},
      {Src0, 1},
      {Src1, 1},
      {Src2, 1},
      {Src3, 1},
      {Vec4, 4},
      {Label, 1},
      {Bool, 1},
      {Integer, 1},
      {UsageToken, 1},
      {LoopCounter, 1},
      {VaradicOperandCount, 0}
    };

    const OperandInfo* lookupOperandInfo(OperandType type) {
      for (OperandInfo& info : operandInfos) {
        if (info.type == type)
          return &info;
      }

      return nullptr;
    }

  }

}