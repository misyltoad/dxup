#pragma once

#include "dx9asm_meta.h"
#include "../util/log.h"
#include "dx9asm_util.h"
#include <vector>

namespace dxapex {

  namespace dx9asm {

    namespace optype {
      enum OperandType {
        Dst,
        Src0,
        Src1,
        Src2,
        Vec4,
        Label,
        Bool,
        Integer,
        UsageToken,
        LoopCounter,
        VaradicOperandCount,
        Count
      };
    }
    using OperandType = optype::OperandType;

    namespace opclass {
      enum OperandClass {
        Register,
        Vec4
      };
    }
    using OperandClass = opclass::OperandClass;

    struct OperandInfo {
      OperandType type;
      uint32_t sizeInTokens;

      inline bool isRegister() const {
        using namespace optype;
        return type == Dst || type == Src0 || type == Src1 || type == Src2;
      }

      inline bool isConstant() const {
        using namespace optype;
        return type == Vec4;
      }

      inline bool isSrc() const {
        using namespace optype;
        return type == Src0 || type == Src1 || type == Src2;
      }

      inline bool isDst() const {
        using namespace optype;
        return type == Dst;
      }
    };

    const OperandInfo* lookupOperandInfo(OperandType type);

    class DX9Operand {
    public:
      DX9Operand(const OperandInfo* info, const uint32_t* tokens)
        : m_info{ info } {
        std::memcpy(m_dx9tokens, tokens, sizeof(uint32_t) * info->sizeInTokens);
      }

      inline OperandType getType() const {
        return m_info->type;
      }

      inline uint32_t getSizeInTokens() const {
        return m_info->sizeInTokens;
      }

      inline const OperandInfo* getInfo() const {
        return m_info;
      }

      inline uint32_t getRegNumber() const {
        return getToken() & D3DSP_REGNUM_MASK;
      }

      inline D3DSHADER_PARAM_REGISTER_TYPE getRegType() const {
        return (D3DSHADER_PARAM_REGISTER_TYPE)(((getToken() & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2) | ((getToken() & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT));
      }

      inline D3DDECLUSAGE getUsage() const {
        return (D3DDECLUSAGE) (getToken() & D3DSP_DCL_USAGEINDEX_MASK);
      }

      inline uint32_t getUsageIndex() const {
        return (getToken() & D3DSP_DCL_USAGEINDEX_MASK) >> D3DSP_DCL_USAGEINDEX_SHIFT;
      }

      inline D3DSHADER_PARAM_SRCMOD_TYPE getModifier() const {
        if (!isSrc()) {
          log::warn("You can't get the modifier of a dst register!");
          return D3DSPSM_NONE;
        }
        return (D3DSHADER_PARAM_SRCMOD_TYPE) (getToken() & D3DSP_SRCMOD_MASK);
      }

      inline uint32_t getSwizzleData() const {
        if (!isSrc()) {
          log::warn("Swizzle data requested for dst register!");
          return 0;
        }

        return getToken() & D3DVS_SWIZZLE_MASK;
      }

      inline uint32_t getWriteMaskData() const {
        if (!isDst()) {
          log::warn("Writemask data requested for src register!");
          return 0;
        }

        return getToken() & D3DSP_WRITEMASK_ALL;
      }

      inline bool getSaturate() const {
        if (!isDst()) {
          log::warn("You can't get the saturation of a src register!");
          return false;
        }

        return getToken() & D3DSPDM_SATURATE;
      }

      inline bool isSrc() const {
        return getInfo()->isSrc();
      }

      inline bool isDst() const {
        return getInfo()->isDst();
      }

      inline bool isRegister() const {
        return getInfo()->isRegister();
      }

      inline void getValues(float* asFloats) const {
        std::memcpy(asFloats, m_dx9tokens, 4);
      }

      inline void getValues(uint32_t* asTokens) const {
        std::memcpy(asTokens, m_dx9tokens, 4);
      }
    private:

      inline uint32_t getToken(uint32_t index = 0) const {
        return m_dx9tokens[index];
      }

      uint32_t m_dx9tokens[4];
      const OperandInfo* m_info;
    };

  }

}