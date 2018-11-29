#include "dx9asm_util.h"
#include "dx9asm_operand.h"
#include "dx9asm_operations.h"
#include "dx9asm_translator.h"
#include "dxbc_helpers.h"

namespace dxapex {

  namespace dx9asm {

    uint32_t byteCodeLength(const uint32_t* dx9asm) {
      if (dx9asm == nullptr)
        return 0;

      const uint32_t* start = dx9asm;
      while (*dx9asm != D3DPS_END())
        dx9asm++;

      return (uint32_t)( (size_t)dx9asm - (size_t)start );
    }

    uint32_t opcode(uint32_t token) {
      return token & D3DSI_OPCODE_MASK;
    }

    void convertRegTypeAndNum(ShaderCodeTranslator& state, DXBCOperand& dstOperand, const DX9Operand& operand, uint32_t regOffset) {
      const RegisterMapping* mapping = state.getRegisterMapping(operand.getRegType(), operand.getRegNumber() + regOffset);
      if (mapping != nullptr) {
        std::memcpy(&dstOperand, &mapping->dxbcOperand, sizeof(DXBCOperand));
        return;
      }

      RegisterMapping newMapping;
      newMapping.dx9Id = operand.getRegNumber() + regOffset;
      newMapping.dx9Type = operand.getRegType();

      newMapping.dxbcOperand.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      newMapping.dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_1D);
      newMapping.dxbcOperand.setExtension(false);
      uint32_t dummyId = 0;
      newMapping.dxbcOperand.setData(&dummyId, 0);

      uint32_t dxbcType = 0;

      // LUT later!
      switch (operand.getRegType()) {
      case D3DSPR_TEMP: dxbcType = D3D10_SB_OPERAND_TYPE_TEMP; break;
      case D3DSPR_INPUT: dxbcType = D3D10_SB_OPERAND_TYPE_INPUT; break;
      case D3DSPR_CONST: {
        const uint32_t constantBufferIndex = 0;
        uint32_t dataWithDummyId[2] = { constantBufferIndex, 0 };
        newMapping.dxbcOperand.setData(dataWithDummyId, 2);
        newMapping.dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_2D);
        newMapping.dxbcOperand.setRepresentation(1, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
        dxbcType = D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER; } break;
      case D3DSPR_ADDR: dxbcType = D3D10_SB_OPERAND_TYPE_RESOURCE; break;
      case D3DSPR_RASTOUT:
      case D3DSPR_ATTROUT:
      case D3DSPR_TEXCRDOUT: dxbcType = D3D10_SB_OPERAND_TYPE_OUTPUT; break;
      case D3DSPR_CONSTINT:
      case D3DSPR_COLOROUT:
      case D3DSPR_DEPTHOUT:
      case D3DSPR_SAMPLER:
      case D3DSPR_CONST2:
      case D3DSPR_CONST3:
      case D3DSPR_CONST4:
      case D3DSPR_CONSTBOOL:
      case D3DSPR_LOOP:
      case D3DSPR_TEMPFLOAT16:
      case D3DSPR_MISCTYPE:
      case D3DSPR_LABEL:
      case D3DSPR_PREDICATE:
      default:
        log::fail("Invalid Register Type"); break;
      }
      
      newMapping.dxbcOperand.setRegisterType(dxbcType);

      state.addRegisterMapping(true, newMapping);

      return convertRegTypeAndNum(state, dstOperand, operand, regOffset);
    }

  }

}