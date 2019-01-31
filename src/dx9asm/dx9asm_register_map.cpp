#include "dx9asm_translator.h"
#include "dx9asm_modifiers.h"
#include "dx9asm_operations.h"
#include "../util/config.h"
#include <unordered_map>

namespace dxup {

  namespace dx9asm {

    RegisterMapping* RegisterMap::lookupOrCreateRegisterMapping(const ShaderCodeTranslator& translator, const DX9Operand& operand, uint32_t regOffset) {
      uint32_t writeMask = 0;
      uint32_t readMask = 0;

      if (operand.isDst())
        writeMask |= calcWriteMask(operand);

      if (operand.isSrc())
        readMask |= calcReadMask(operand);

      return lookupOrCreateRegisterMapping(translator, operand.getRegType(), operand.getRegNumber() + regOffset, readMask, writeMask);
    }

    std::vector<TransientRegisterMapping> transientMappings = {
      {0, 0, D3DDECLUSAGE_POSITION, "SV_Position", 0},

      // SM1 Up me later!
      {1, 0, D3DDECLUSAGE_TEXCOORD, "TEXCOORD", 0},
      {2, 1, D3DDECLUSAGE_TEXCOORD, "TEXCOORD", 1},
      {3, 2, D3DDECLUSAGE_TEXCOORD, "TEXCOORD", 2},
      {4, 3, D3DDECLUSAGE_TEXCOORD, "TEXCOORD", 3},
      {5, 4, D3DDECLUSAGE_TEXCOORD, "TEXCOORD", 4},
      {6, 5, D3DDECLUSAGE_TEXCOORD, "TEXCOORD", 5},
      {7, 6, D3DDECLUSAGE_TEXCOORD, "TEXCOORD", 6},
      {8, 7, D3DDECLUSAGE_TEXCOORD, "TEXCOORD", 7},

      {9, 0, D3DDECLUSAGE_COLOR, "TEXCOORD", 8},
      {10, 1, D3DDECLUSAGE_COLOR, "TEXCOORD", 9},

      {11, 0, D3DDECLUSAGE_FOG, "TEXCOORD", 10},
      {12, 0, D3DDECLUSAGE_PSIZE, "TEXCOORD", 11},
    };

    std::vector<TransientRegisterMapping>& RegisterMap::getTransientMappings() {
      return transientMappings;
    }

    uint32_t RegisterMap::getTransientId(DclInfo& info) {
      for (const TransientRegisterMapping& mapping : transientMappings) {
        if (mapping.d3d9Usage == info.usage) {
          if (mapping.d3d9UsageIndex == info.usageIndex)
            return mapping.dxbcRegNum;
        }
      }

      log::warn("Unable to find transient register! Creating a new transient mapping:\nUsage: %lu\nUsage Index: %lu", info.usage, info.usageIndex);

      TransientRegisterMapping mapping;
      mapping.d3d9Usage = info.usage;
      mapping.d3d9UsageIndex = info.usageIndex;
      mapping.dxbcRegNum = transientMappings.size();

      transientMappings.push_back(mapping);

      return mapping.dxbcRegNum;
    }

    RegisterMapping* RegisterMap::lookupOrCreateRegisterMapping(const ShaderCodeTranslator& translator, uint32_t regType, uint32_t regNum, uint32_t readMask, uint32_t writeMask, bool readingLikeVSOutput) {
      RegisterMapping* mapping = getRegisterMapping(regType, regNum);

      if (mapping != nullptr) {
        mapping->writeMask |= writeMask;
        //mapping->readMask |= readMask;

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

      uint32_t dxbcType = 0;

      // This may get set later depending on the following stuff.
      newMapping.dxbcOperand.setData(&newMapping.dx9Id, 1);

      // LUT later!
      switch (regType) {
      case D3DSPR_TEMP:
        dxbcType = D3D10_SB_OPERAND_TYPE_TEMP; break;

      case D3DSPR_INPUT: {
        dxbcType = D3D10_SB_OPERAND_TYPE_INPUT;

        if (translator.getMajorVersion() != 3 && translator.getShaderType() == ShaderType::Pixel) {
          newMapping.dclInfo.type = UsageType::Input;
          newMapping.dclInfo.usage = D3DDECLUSAGE_COLOR;
          newMapping.dclInfo.usageIndex = newMapping.dx9Id;
        }
      } break;

      case D3DSPR_CONSTBOOL:
      case D3DSPR_CONSTINT:
      case D3DSPR_CONST: {

        const uint32_t constantBufferIndex = 0;
        uint32_t constId = newMapping.dx9Id;
        if (regType == D3DSPR_CONSTINT)
          constId += 256;
        else if (regType == D3DSPR_CONSTBOOL)
          constId += 256 + 16;

        uint32_t dataWithDummyId[2] = { constantBufferIndex, constId };
        newMapping.dxbcOperand.setData(dataWithDummyId, 2);
        newMapping.dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_2D);
        newMapping.dxbcOperand.setRepresentation(1, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
        dxbcType = D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER;

      } break;

      case D3DSPR_RASTOUT: {
        newMapping.dclInfo.type = readingLikeVSOutput ? UsageType::Input : UsageType::Output;

        if (newMapping.dx9Id == D3DSRO_POSITION)
          newMapping.dclInfo.usage = D3DDECLUSAGE_POSITION;
        else if (newMapping.dx9Id == D3DSRO_FOG)
          newMapping.dclInfo.usage = D3DDECLUSAGE_FOG;
        else if (newMapping.dx9Id == D3DSRO_POINT_SIZE)
          newMapping.dclInfo.usage = D3DDECLUSAGE_PSIZE;

        newMapping.dclInfo.usageIndex = 0;

        dxbcType = readingLikeVSOutput ? D3D10_SB_OPERAND_TYPE_INPUT : D3D10_SB_OPERAND_TYPE_OUTPUT;

      } break;

      case D3DSPR_TEXCRDOUT: { // D3DSPR_OUTPUT
        newMapping.dclInfo.type = readingLikeVSOutput ? UsageType::Input : UsageType::Output;
        newMapping.dclInfo.usage = D3DDECLUSAGE_TEXCOORD;
        newMapping.dclInfo.usageIndex = newMapping.dx9Id;

        dxbcType = readingLikeVSOutput ? D3D10_SB_OPERAND_TYPE_INPUT : D3D10_SB_OPERAND_TYPE_OUTPUT;
      } break;

      case D3DSPR_ATTROUT: {

        newMapping.dclInfo.type = readingLikeVSOutput ? UsageType::Input : UsageType::Output;
        newMapping.dclInfo.usage = D3DDECLUSAGE_COLOR;
        newMapping.dclInfo.usageIndex = newMapping.dx9Id;

        dxbcType = readingLikeVSOutput ? D3D10_SB_OPERAND_TYPE_INPUT : D3D10_SB_OPERAND_TYPE_OUTPUT;
        break;
      }

      case D3DSPR_ADDR: { // D3DSPR_TEXTURE
        if (translator.getShaderType() == ShaderType::Pixel) {

          // SM2 or 1.4
          if (translator.getMajorVersion() >= 2 || (translator.getMajorVersion() == 1 && translator.getMinorVersion() == 4)) {
            newMapping.dclInfo.type = UsageType::Input;
            newMapping.dclInfo.usage = D3DDECLUSAGE_TEXCOORD;
            newMapping.dclInfo.usageIndex = newMapping.dx9Id;
            dxbcType = D3D10_SB_OPERAND_TYPE_INPUT;
          }
          else
            dxbcType = D3D10_SB_OPERAND_TYPE_TEMP;
        }
        else {
          dxbcType = D3D10_SB_OPERAND_TYPE_TEMP;
        }

        break;
      }

      case D3DSPR_COLOROUT: {
        newMapping.dclInfo.type = readingLikeVSOutput ? UsageType::Input : UsageType::Output;
        newMapping.dclInfo.target = true;
        newMapping.dclInfo.usage = D3DDECLUSAGE_COLOR;
        newMapping.dclInfo.usageIndex = newMapping.dx9Id;
        dxbcType = readingLikeVSOutput ? D3D10_SB_OPERAND_TYPE_INPUT : D3D10_SB_OPERAND_TYPE_OUTPUT;
        break;
      }

      case D3DSPR_DEPTHOUT:
      case D3DSPR_SAMPLER:
      case D3DSPR_CONST2:
      case D3DSPR_CONST3:
      case D3DSPR_CONST4:
      case D3DSPR_LOOP:
      case D3DSPR_TEMPFLOAT16:
      case D3DSPR_MISCTYPE:
      case D3DSPR_LABEL:
      case D3DSPR_PREDICATE:
      default:
        log::fail("Invalid Register Type"); break;
      }

      newMapping.dxbcOperand.setRegisterType(dxbcType);

      bool io = newMapping.dclInfo.type != UsageType::None;
      bool transient = io && translator.isTransient(newMapping.dclInfo.type == UsageType::Input);
      bool generateId = translator.shouldGenerateId(transient);

      if (dxbcType == D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER)
        generateId = false;

      addRegisterMapping(transient, generateId, newMapping);

      return lookupOrCreateRegisterMapping(translator, regType, regNum, readMask, writeMask);
    }

  }

}