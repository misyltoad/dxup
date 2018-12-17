#include "dx9asm_translator.h"
#include "dx9asm_modifiers.h"
#include "dx9asm_operations.h"
#include "../util/config.h"
#include <unordered_map>

namespace dxapex {

  namespace dx9asm {

    RegisterMapping* RegisterMap::lookupOrCreateRegisterMapping(ShaderType type, uint32_t majorVersion, uint32_t minorVersion, const DX9Operand& operand, uint32_t regOffset) {
      uint32_t writeMask = 0;
      uint32_t readMask = 0;

      if (operand.isDst())
        writeMask |= calcWriteMask(operand);

      if (operand.isSrc())
        readMask |= calcReadMask(operand);

      return lookupOrCreateRegisterMapping(type, majorVersion, minorVersion, operand.getRegType(), operand.getRegNumber() + regOffset, readMask, writeMask);
    }

    struct TransientRegisterMapping {
      uint32_t regNum;
      uint32_t d3d9Usage;
    };
    std::vector<TransientRegisterMapping> transientMappings = {
      {0, D3DDECLUSAGE_POSITION},

      {1, D3DDECLUSAGE_COLOR},
      {2, D3DDECLUSAGE_COLOR},

      // SM1 Up me later!
      {3, D3DDECLUSAGE_TEXCOORD},
      {4, D3DDECLUSAGE_TEXCOORD},
      {5, D3DDECLUSAGE_TEXCOORD},
      {6, D3DDECLUSAGE_TEXCOORD},

      {7, D3DDECLUSAGE_FOG},
      {8, D3DDECLUSAGE_PSIZE},
    };

    uint32_t RegisterMap::getTransientId(DclInfo& info) {
      uint32_t countOfMyType = 0;
      static uint32_t lastUnimplementedTransientId = 14;

      for (const TransientRegisterMapping& mapping : transientMappings) {
        if (mapping.d3d9Usage == info.usage) {
          if (countOfMyType == info.usageIndex)
            return mapping.regNum;

          countOfMyType++;
        }
      }

      log::warn("Unable to find transient register!");
      return lastUnimplementedTransientId++;
    }

    RegisterMapping* RegisterMap::lookupOrCreateRegisterMapping(ShaderType type, uint32_t minorVersion, uint32_t majorVersion, uint32_t regType, uint32_t regNum, uint32_t readMask, uint32_t writeMask) {
      RegisterMapping* mapping = getRegisterMapping(regType, regNum);

      if (mapping != nullptr) {
        mapping->writeMask |= writeMask;
        mapping->readMask |= readMask;

        return mapping;
      }

      RegisterMapping newMapping;
      newMapping.dclInfo.type = UsageType::None;
      newMapping.dx9Id = regNum;
      newMapping.dx9Type = regType;
      newMapping.writeMask = writeMask;
      newMapping.readMask = readMask;

      newMapping.dxbcOperand.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      newMapping.dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_1D);
      newMapping.dxbcOperand.stripModifier();
      uint32_t dummyId = 0;
      newMapping.dxbcOperand.setData(&dummyId, 1);

      uint32_t dxbcType = 0;
      bool addMapping = true;

      // LUT later!
      switch (regType) {
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
        newMapping.dclInfo.type = type == ShaderType::Pixel ? UsageType::Input : UsageType::Output;
        newMapping.dclInfo.usage = D3DDECLUSAGE_TEXCOORD;
        newMapping.dclInfo.usageIndex = newMapping.dx9Id;

        dxbcType = type == ShaderType::Pixel ? D3D10_SB_OPERAND_TYPE_INPUT : D3D10_SB_OPERAND_TYPE_OUTPUT;
      } break;
      case D3DSPR_ATTROUT: {
        newMapping.dclInfo.type = UsageType::Output;
        newMapping.dclInfo.usage = D3DDECLUSAGE_COLOR;
        newMapping.dclInfo.usageIndex = newMapping.dx9Id;

        dxbcType = D3D10_SB_OPERAND_TYPE_OUTPUT;
      } break;
      case D3DSPR_ADDR: {
        if (type == ShaderType::Pixel) {
          dxbcType = D3D10_SB_OPERAND_TYPE_TEMP;

          break;
        }
      }
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

      bool transient = (type == ShaderType::Pixel && newMapping.dclInfo.type == UsageType::Input) ||
                       (type == ShaderType::Vertex && newMapping.dclInfo.type == UsageType::Output);

      addRegisterMapping(transient, true, newMapping);

      return lookupOrCreateRegisterMapping(type, minorVersion, majorVersion, regType, regNum, readMask, writeMask);
    }

  }

}