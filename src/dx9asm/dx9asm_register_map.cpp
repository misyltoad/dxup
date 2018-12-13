#include "dx9asm_translator.h"
#include "dx9asm_modifiers.h"
#include "dx9asm_operations.h"
#include "../util/config.h"

namespace dxapex {

  namespace dx9asm {

    RegisterMapping* RegisterMap::lookupOrCreateRegisterMapping(ShaderType type, uint32_t majorVersion, uint32_t minorVersion, const DX9Operand& operand, uint32_t regOffset) {
      RegisterMapping* mapping = getRegisterMapping(operand.getRegType(), operand.getRegNumber() + regOffset);

      uint32_t writeMask = 0;
      uint32_t readMask = 0;

      if (operand.isDst())
        writeMask |= calcWriteMask(operand);

      if (operand.isSrc())
        readMask |= calcReadMask(operand);

      if (operand.isSrc())
        readMask |= calcReadMask(operand);

      if (mapping != nullptr) {
        mapping->writeMask |= writeMask;
        mapping->readMask |= readMask;

        return mapping;
      }

      RegisterMapping newMapping;
      newMapping.dclInfo.type = UsageType::None;
      newMapping.dx9Id = operand.getRegNumber() + regOffset;
      newMapping.dx9Type = operand.getRegType();
      newMapping.writeMask = writeMask;
      newMapping.readMask = readMask;

      newMapping.dxbcOperand.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      newMapping.dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_1D);
      newMapping.dxbcOperand.setExtension(false);
      uint32_t dummyId = 0;
      newMapping.dxbcOperand.setData(&dummyId, 1);

      uint32_t dxbcType = 0;

      // LUT later!
      switch (operand.getRegType()) {
      case D3DSPR_TEMP: { 
        if (type == ShaderType::Pixel && majorVersion == 1) {
          if (newMapping.dx9Id == 0) {
            // Pixel Shader...
            newMapping.dclInfo.type = UsageType::Output;
            newMapping.dclInfo.target = true;
            newMapping.dclInfo.usage = (D3DDECLUSAGE)0;
            newMapping.dclInfo.usageIndex = newMapping.dx9Id;

            dxbcType = D3D10_SB_OPERAND_TYPE_OUTPUT;
            break;
          }
        }

        dxbcType = D3D10_SB_OPERAND_TYPE_TEMP; break;
      }
      case D3DSPR_INPUT: dxbcType = D3D10_SB_OPERAND_TYPE_INPUT; break;
      case D3DSPR_CONST: {
        const uint32_t constantBufferIndex = 0;
        uint32_t dataWithDummyId[2] = { constantBufferIndex, 0 };
        newMapping.dxbcOperand.setData(dataWithDummyId, 2);
        newMapping.dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_2D);
        newMapping.dxbcOperand.setRepresentation(1, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
        dxbcType = D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER; } break;
      case D3DSPR_RASTOUT: {
        newMapping.dclInfo.type = UsageType::Output;

        if (newMapping.dx9Id == D3DSRO_POSITION)
          newMapping.dclInfo.usage = D3DDECLUSAGE_POSITION;
        else if (newMapping.dx9Id == D3DSRO_FOG)
          newMapping.dclInfo.usage = D3DDECLUSAGE_FOG;
        else if (newMapping.dx9Id == D3DSRO_POINT_SIZE)
          newMapping.dclInfo.usage = D3DDECLUSAGE_PSIZE;

        newMapping.dclInfo.usageIndex = 0;

        dxbcType = D3D10_SB_OPERAND_TYPE_OUTPUT;

      } break;
      case D3DSPR_TEXCRDOUT: {
        newMapping.dclInfo.type = UsageType::Output;
        newMapping.dclInfo.usage = D3DDECLUSAGE_TEXCOORD;
        newMapping.dclInfo.usageIndex = newMapping.dx9Id;

        dxbcType = D3D10_SB_OPERAND_TYPE_OUTPUT;
      } break;
      case D3DSPR_ATTROUT: {
        newMapping.dclInfo.type = UsageType::Output;
        newMapping.dclInfo.usage = D3DDECLUSAGE_COLOR;
        newMapping.dclInfo.usageIndex = newMapping.dx9Id;

        dxbcType = D3D10_SB_OPERAND_TYPE_OUTPUT;
      } break;
      case D3DSPR_ADDR:
      case D3DSPR_COLOROUT:
      case D3DSPR_CONSTINT:
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

      addRegisterMapping(true, newMapping);

      return lookupOrCreateRegisterMapping(type, minorVersion, majorVersion, operand, regOffset);
    }

  }

}