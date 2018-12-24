#include "dx9asm_operand.h"

namespace dxup {

  namespace dx9asm {

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