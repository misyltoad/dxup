#include "dx9asm_translator.h"
#include "dx9asm_modifiers.h"
#include "../util/fourcc.h"
#include "../util/config.h"

namespace dxapex {

  namespace dx9asm {

    // Varadic Operand
    bool ShaderCodeTranslator::handleComment(DX9Operation& operation) {
      uint32_t commentTokenCount = operation.getCommentCount();

      uint32_t cc = nextToken();
      if (cc == fourcc("CTAB")) {
        m_ctab = (CTHeader*)(m_head);
        uint32_t tableSize = (commentTokenCount - 1) * sizeof(uint32_t);

        if (tableSize < sizeof(CTHeader) || m_ctab->size != sizeof(CTHeader)) {
          log::fail("CTAB invalid!");
          return false; // fatal
        }
      }

      skipTokens(commentTokenCount - 1);
      return true;
    }

    bool ShaderCodeTranslator::handleDcl(DX9Operation& operation) {
      const DX9Operand* usageToken = operation.getOperandByType(optype::UsageToken);
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);

      RegisterMapping* mapping = m_map.lookupOrCreateRegisterMapping(getShaderType(), getMajorVersion(), getMinorVersion(), *dst);

      if (dst->getRegType() == D3DSPR_INPUT)
        mapping->dclInfo.type = UsageType::Input;
      else if (dst->getRegType() == D3DSPR_OUTPUT)
        mapping->dclInfo.type = UsageType::Output;
      else {
        log::warn("Unhandled reg type in dcl.");
        mapping->dclInfo.type = UsageType::Output;
      }
      mapping->dclInfo.usage = usageToken->getUsage();
      mapping->dclInfo.usageIndex = usageToken->getUsageIndex();
      mapping->dclInfo.centroid = dst->centroid();
      
      return true;
    }

    // Varadic Operand
    bool ShaderCodeTranslator::handleTex(DX9Operation& operation) {
      // SM1_1 impl for now.

      DX9Operand dst{ lookupOperandInfo(optype::Dst), nextToken() };
      uint32_t regNum = dst.getRegNumber();

      m_samplerMask |= 1 << regNum;

      // Fake a D3D9 texcoord register.
      RegisterMapping* texCoordMapping = m_map.lookupOrCreateRegisterMapping(
        getShaderType(),
        getMajorVersion(),
        getMinorVersion(),
        D3DSPR_TEXCRDOUT,
        regNum, 
        ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE) | ENCODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(D3D10_SB_OPERAND_4_COMPONENT_MASK_X | D3D10_SB_OPERAND_4_COMPONENT_MASK_Y),
        0);

      DXBCOperand texCoordOp = texCoordMapping->dxbcOperand;
      texCoordOp.setSwizzleOrWritemask(
        ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE) | 
        ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE(D3D10_SB_4_COMPONENT_X, D3D10_SB_4_COMPONENT_Y, D3D10_SB_4_COMPONENT_X, D3D10_SB_4_COMPONENT_X)
      );

      RegisterMapping* dstMapping = m_map.lookupOrCreateRegisterMapping(getShaderType(), getMajorVersion(), getMinorVersion(), dst);
      DXBCOperand dstOp = dstMapping->dxbcOperand;
      dstOp.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      calculateDXBCSwizzleAndWriteMask(dstOp, dst);
      calculateDXBCModifiers(dstOp, operation, dst);

      DXBCOperand textureOp{ D3D10_SB_OPERAND_TYPE_RESOURCE, 1 };
      textureOp.setData(&regNum, 1);
      textureOp.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      textureOp.setSwizzleOrWritemask(noSwizzle);

      DXBCOperand samplerOp{ D3D10_SB_OPERAND_TYPE_SAMPLER, 1 };
      samplerOp.setData(&regNum, 1);
      samplerOp.setComponents(0);
      samplerOp.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);

      DXBCOperation{D3D10_SB_OPCODE_SAMPLE, false}
        .setSampler(true)
        .appendOperand(dstOp)
        .appendOperand(texCoordOp)
        .appendOperand(textureOp)
        .appendOperand(samplerOp)
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handleDef(DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* vec4 = operation.getOperandByType(optype::Vec4);

      RegisterMapping mapping;
      mapping.dx9Id = dst->getRegNumber();
      mapping.dx9Type = dst->getRegType();
      mapping.dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_IMMEDIATE32);
      mapping.dxbcOperand.stripModifier();
      mapping.dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_0D);
      uint32_t data[4];
      vec4->getValues(data);
      mapping.dxbcOperand.setData(data, 4);
      mapping.dxbcOperand.setupLiteral(4);

      mapping.dxbcOperand.setSwizzleOrWritemask(noSwizzle);

      m_map.addRegisterMapping(false, false, mapping);

      return true;
    }

    bool ShaderCodeTranslator::handleUniqueOperation(DX9Operation& operation) {
      UniqueFunction function = operation.getUniqueFunction();
      if (function == nullptr) {
        log::fail("Unimplemented operation %s encountered.", operation.getName());
        return !config::getBool(config::UnimplementedFatal);
      }

      return std::invoke(function, this, operation);
    }

  }

}