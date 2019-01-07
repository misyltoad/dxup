#include "dx9asm_translator.h"
#include "dx9asm_modifiers.h"
#include "dx9asm_operations.h"
#include "../util/config.h"
#include <unordered_map>

namespace dxup {

  namespace dx9asm {

    RegisterMapping* RegisterMap::lookupOrCreateRegisterMapping(const ShaderCodeTranslator& translator, const DX9Operand& operand, uint32_t regOffset) {
      uint32_t mask = 0;

      if (operand.isDst())
        mask |= calcWriteMask(operand);
      else if (operand.isSrc())
        mask |= calcReadMask(operand);

      return lookupOrCreateRegisterMapping(translator, { operand.getRegType(), operand.getRegNumber() + regOffset }, mask);
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

    uint32_t RegisterMap::getTransientId(const DclInfo& info) {
      for (const TransientRegisterMapping& mapping : transientMappings) {
        if (mapping.d3d9Usage == info.getUsage()) {
          if (mapping.d3d9UsageIndex == info.getUsageIndex())
            return mapping.dxbcRegNum;
        }
      }

      log::warn("Unable to find transient register! Creating a new transient mapping:\nUsage: %lu\nUsage Index: %lu", info.getUsage(), info.getUsageIndex());

      TransientRegisterMapping mapping;
      mapping.d3d9Usage = info.getUsage();
      mapping.d3d9UsageIndex = info.getUsageIndex();
      mapping.dxbcRegNum = transientMappings.size();

      transientMappings.push_back(mapping);

      return mapping.dxbcRegNum;
    }

    RegisterMapping* RegisterMap::lookupOrCreateRegisterMapping(const ShaderCodeTranslator& translator, RegisterId id, uint32_t mask, uint32_t versionOverride) {
      RegisterMapping* mapping = getRegisterMapping(id);

      if (mapping != nullptr) {
        mapping->addToMask(mask);

        return mapping;
      }

      DXBCOperand dxbcOperand;

      dxbcOperand.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_1D);

      // This may get set later depending on the following stuff.
      {
        uint32_t dx9Id = id.getRegNum();
        dxbcOperand.setData(&dx9Id, 1);
      }

      DclInfo dclInfo;

      switch (id.getRegType()) {
      case D3DSPR_TEMP:
        dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_TEMP); break;

      case D3DSPR_INPUT: {
        dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_INPUT);

        if (translator.getMajorVersion() != 3 && translator.getShaderType() == ShaderType::Pixel)
          dclInfo = DclInfo{ D3DDECLUSAGE_COLOR, id.getRegNum(), dclFlags::input };
      } break;

      case D3DSPR_CONSTINT:
      case D3DSPR_CONST: {

        const uint32_t constantBufferIndex = 0;
        uint32_t fullId = id.getRegNum();
        if (id.getRegType() == D3DSPR_CONSTINT)
          fullId += 256;

        uint32_t dataWithDummyId[2] = { constantBufferIndex, fullId };
        dxbcOperand.setData(dataWithDummyId, 2);
        dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_2D);
        dxbcOperand.setRepresentation(1, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
        dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER);

      } break;

      case D3DSPR_RASTOUT: {
        D3DDECLUSAGE usage = D3DDECLUSAGE_POSITION;
        if (id.getRegNum() == D3DSRO_FOG)
          usage = D3DDECLUSAGE_FOG;
        else if (id.getRegNum() == D3DSRO_POINT_SIZE)
          usage = D3DDECLUSAGE_PSIZE;

        dclInfo = DclInfo{ usage, 0, dclFlags::output };

        dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_OUTPUT);

      } break;

      case D3DSPR_TEXCRDOUT: { // D3DSPR_OUTPUT
        dclInfo = DclInfo{ D3DDECLUSAGE_TEXCOORD, id.getRegNum(), dclFlags::output };

        dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_OUTPUT);
      } break;

      case D3DSPR_ATTROUT: {
        dclInfo = DclInfo{ D3DDECLUSAGE_COLOR, id.getRegNum(), dclFlags::output };

        dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_OUTPUT);
        break;
      }

      case D3DSPR_ADDR: { // D3DSPR_TEXTURE

        // Texcoord/Tex Register

        bool input = translator.getMajorVersion() >= 2 || (translator.getMajorVersion() == 1 && translator.getMinorVersion() == 4);
        input = input || versionOverride >= 2;
        input = input && translator.getShaderType() == ShaderType::Pixel;

        if (input) {
          dclInfo = DclInfo{ D3DDECLUSAGE_TEXCOORD, id.getRegNum(), dclFlags::input };
          dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_INPUT);
        }
        else
          dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_TEMP); // This changes value w/ tex/texcoord operands.

        break;
      }

      case D3DSPR_COLOROUT: {
        uint32_t dclFlags = dclFlags::output;
        if (translator.getShaderType() == ShaderType::Pixel)
          dclFlags |= dclFlags::target;

        dclInfo = DclInfo{ D3DDECLUSAGE_COLOR, id.getRegNum(), dclFlags };
        dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_OUTPUT);
        break;
      }

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
        log::fail("Unsupported register type: %d", id.getRegType()); break;
      }

      bool shouldGenerateId = true;
      bool transient = dclInfo.isValid() && translator.isTransient(dclInfo.isInput());

      if (dxbcOperand.getRegisterType() == D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER)//&& translator.isIndirectMarked())
        shouldGenerateId = false;

      if (shouldGenerateId)
        generateId(translator.isTransient(dclInfo.isInput()), dxbcOperand, dclInfo);

      addRegisterMapping(id, RegisterMapping{ dxbcOperand, mask, dclInfo });

      return lookupOrCreateRegisterMapping(translator, id, mask);
    }

  }

}