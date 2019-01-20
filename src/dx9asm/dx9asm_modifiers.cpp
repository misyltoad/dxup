#include "dx9asm_modifiers.h"
#include "dx9asm_operations.h"
#include "dx9asm_translator.h"
#include "dxbc_helpers.h"
#include "../util/log.h"

namespace dxup {

  namespace dx9asm {

    using namespace implicitflag;

/*    struct ModifierMap {
      D3DSHADER_PARAM_SRCMOD_TYPE srcModifier;
      uint32_t equivelantImplicitFlags;
    };

    ModifierMap*/
    // Replace with LUT

    void calculateDXBCModifiers(DXBCOperand& dstOperand, const DX9Operation& operation, const DX9Operand& operand) {
      const DX9ToDXBCImplicitConversionInfo& implicitOpInfo = operation.getImplicitInfo();

      if (!operand.isRegister()) {
        log::fail("Attempting to calculate modifiers on a non-register operand.");
        return;
      }

      if (operand.isSrc()) {
        uint32_t modifier = operand.getModifier();

        switch (modifier) {
        case D3DSPSM_NONE: {

          if (implicitOpInfo.implicitFlags & abs && implicitOpInfo.implicitFlags & negate) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABSNEG);
            break;
          }
          else if (implicitOpInfo.implicitFlags & abs) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABS);
            break;
          }
          else if (implicitOpInfo.implicitFlags & negate) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_NEG);
            break;
          }
          else {
            dstOperand.stripModifier();
            break;
          }

        }

        case D3DSPSM_NEG: {

          if (implicitOpInfo.implicitFlags & abs && implicitOpInfo.implicitFlags & negate) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABS);
            break;
          }
          else if (implicitOpInfo.implicitFlags & abs) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABSNEG);
            break;
          }
          else if (implicitOpInfo.implicitFlags & negate) {
            dstOperand.stripModifier();
            break;
          }
          else {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_NEG);
            break;
          }

        }

        case D3DSPSM_ABS: {

          if (implicitOpInfo.implicitFlags & abs && implicitOpInfo.implicitFlags & negate) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABSNEG);
            break;
          }
          else if (implicitOpInfo.implicitFlags & abs) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABS);
            break;
          }
          else if (implicitOpInfo.implicitFlags & negate) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABSNEG);
            break;
          }
          else {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABS);
            break;
          }

        }
        case D3DSPSM_ABSNEG: {

          if (implicitOpInfo.implicitFlags & abs && implicitOpInfo.implicitFlags & negate) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABSNEG);
            break;
          }
          else if (implicitOpInfo.implicitFlags & abs) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABS);
            break;
          }
          else if (implicitOpInfo.implicitFlags & negate) {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABS);
            break;
          }
          else {
            dstOperand.setModifier(D3D10_SB_OPERAND_MODIFIER_ABSNEG);
            break;
          }

        }

        default: log::fail("Unimplemented operand modifier! (sm1 asm garbage?)"); break;
        }
      }
    }

    uint32_t fixSwizzle(uint32_t numUsed, uint32_t dx9swizzle) {
      uint32_t outSwizzle = 0;

      for (uint32_t i = 0; i < 4; i++) {
        // Move them all into X_* swizzled space.
        uint32_t shift = i * 2;
        bool used = i <= numUsed - 1;

        uint32_t firstSwizzleInMyShift = ((dx9swizzle & 0x00030000) >> shift);
        uint32_t thisSwizzle = (dx9swizzle & (0x00030000 << shift));
        if (used)
          outSwizzle |= thisSwizzle;
        else
          outSwizzle |= firstSwizzleInMyShift;
      }

      return outSwizzle;
    }

    uint32_t calcSwizzle(uint32_t dx9swizzle, uint32_t numComponents) {
      dx9swizzle = fixSwizzle(numComponents, dx9swizzle);

      if (dx9swizzle == D3DVS_NOSWIZZLE)
        return noSwizzle;

      dx9swizzle = dx9swizzle >> D3DVS_SWIZZLE_SHIFT;

      // The swizzle mask is the same as we use in dxbc, just shifted!
      dx9swizzle = dx9swizzle << D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_SHIFT;
      return dx9swizzle | ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE);
    }

    uint32_t calcSwizzle(const DX9Operand& operand) {
      if (!operand.isSrc() || !operand.isRegister())
        return 0;

      return calcSwizzle(operand.getSwizzleData(), operand.getUsedComponents());
    }

    uint32_t calcReadMask(uint32_t dx9swizzle, uint32_t usedComponents) {
      dx9swizzle = fixSwizzle(usedComponents, dx9swizzle);

      uint32_t readMask = 0;

      for (uint32_t i = 0; i < 4; i++) {
        // Move them all into X_* swizzled space.
        uint32_t shift = i * 2;

        uint32_t thisSwizzle = ((dx9swizzle & (0x00030000 << shift)) >> shift);

        if (thisSwizzle == D3DVS_X_X)
          readMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_X;

        if (thisSwizzle == D3DVS_X_Y)
          readMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_Y;

        if (thisSwizzle == D3DVS_X_Z)
          readMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_Z;

        if (thisSwizzle == D3DVS_X_W)
          readMask |= D3D10_SB_OPERAND_4_COMPONENT_MASK_W;
      }

      return readMask;
    }

    uint32_t calcReadMask(const DX9Operand& operand) {
      uint32_t dx9swizzle = operand.getSwizzleData();

      if (!operand.isSrc() || !operand.isRegister())
        return 0;

      return calcReadMask(dx9swizzle, operand.getUsedComponents());
    }

    uint32_t calcWriteMask(const DX9Operand& operand) {
      uint32_t dx9WriteMask = operand.getWriteMaskData();

      if (!operand.isDst() || !operand.isRegister())
        return 0;

      return calcWriteMask(dx9WriteMask);
    }
    uint32_t calcWriteMask(uint32_t dx9WriteMask) {
      if (dx9WriteMask == D3DSP_WRITEMASK_ALL || dx9WriteMask == 0x00000000)
        return writeAll;

      // The write mask is the same as we use in dxbc, just shifted!
      uint32_t dxbcWriteMask = (dx9WriteMask >> 16);
      dxbcWriteMask <<= D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT;

      return dxbcWriteMask | ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE);
    }

    void calculateDXBCSwizzleAndWriteMask(DXBCOperand& dstOperand, const DX9Operand& operand) {
      if (!operand.isRegister()) {
        log::fail("Attempting to calculate swizzle/writemask on a non-register operand.");
        return;
      }

      if (operand.isSrc())
        dstOperand.setSwizzleOrWritemask(calcSwizzle(operand));
      else
        dstOperand.setSwizzleOrWritemask(calcWriteMask(operand));
    }

  }

}