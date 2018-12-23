#include "dx9asm_translator.h"
#include "dx9asm_modifiers.h"
#include "../util/fourcc.h"
#include "../util/config.h"

namespace dxup {

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

      RegisterMapping mapping;
      mapping.dx9Id = dst->getRegNumber();
      mapping.dx9Type = dst->getRegType();
      mapping.readMask |= calcWriteMask(*dst);

      mapping.dxbcOperand.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      mapping.dxbcOperand.setDimension(D3D10_SB_OPERAND_INDEX_1D);
      mapping.dxbcOperand.stripModifier();

      mapping.dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_INPUT);

      if (dst->getRegType() == D3DSPR_INPUT || dst->getRegType() == D3DSPR_TEXTURE)
        mapping.dclInfo.type = UsageType::Input;
      else if (dst->getRegType() == D3DSPR_OUTPUT) {
        mapping.dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_OUTPUT);
        mapping.dclInfo.type = UsageType::Output;
      }
      else if (dst->getRegType() == D3DSPR_SAMPLER) {

        SamplerDesc desc;
        desc.index = dst->getRegNumber();
        switch (dst->getTextureType())
        {
        default:
        case D3DSTT_UNKNOWN:
        case D3DSTT_2D:
          desc.dimension = D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D; break;
        case D3DSTT_CUBE:
          desc.dimension = D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE; break;
        case D3DSTT_VOLUME:
          desc.dimension = D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D; break;
        }

        m_samplers.push_back(desc);
        return true;

      }
      else {
        log::fail("Unhandled reg type in dcl, %d.", dst->getRegType());
        mapping.dclInfo.type = UsageType::Output;
      }
      mapping.dclInfo.usage = usageToken->getUsage();
      mapping.dclInfo.usageIndex = usageToken->getUsageIndex();

      if (getShaderType() == ShaderType::Pixel && mapping.dclInfo.type == UsageType::Output) {
        if (mapping.dclInfo.usage == D3DDECLUSAGE_COLOR)
          mapping.dclInfo.target = true;
        else if (mapping.dclInfo.usage == D3DDECLUSAGE_DEPTH)
          log::fail("Writing to oDepth not supported yet.");
      }

      mapping.dclInfo.centroid = dst->centroid();

      // This may get changed later...
      mapping.dxbcOperand.setData(&mapping.dx9Id, 1);

      bool transient = (getShaderType() == ShaderType::Pixel && mapping.dclInfo.type == UsageType::Input) ||
                       (getShaderType() == ShaderType::Vertex && mapping.dclInfo.type == UsageType::Output);

      bool generateId = !transient || (transient && getMajorVersion() != 3);

      getRegisterMap().addRegisterMapping(transient, generateId, mapping);
      
      return true;
    }

    bool ShaderCodeTranslator::handleNrm(DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);

      DXBCOperand tempOp = getRegisterMap().getNextInternalTemp();
      DXBCOperand srcOp = { *this, operation, *src0, 0 };
      DXBCOperand dstOp = { *this, operation, *dst, 0 };

      // DP with self to get length squared.
      DXBCOperation{ D3D10_SB_OPCODE_DP4, false }
        .appendOperand(tempOp)
        .appendOperand(srcOp)
        .push(*this);

      DXBCOperation{ D3D10_SB_OPCODE_RSQ, false }
        .appendOperand(tempOp)
        .appendOperand(tempOp)
        .push(*this);

      DXBCOperation{ D3D10_SB_OPCODE_MUL, false }
        .appendOperand(srcOp)
        .appendOperand(dstOp)
        .appendOperand(tempOp)
        .push(*this);

      return true;
    }

    // Varadic Operand
    bool ShaderCodeTranslator::handleTex(DX9Operation& operation) {
      DX9Operand dst{ lookupOperandInfo(optype::Dst), nextToken() };
      uint32_t samplerRegNum = dst.getRegNumber();

      DXBCOperand texCoordOp;

      // sm_1.1 - sm1.3
      if (getMajorVersion() == 1 && getMinorVersion() <= 3) {
        // Fake a D3D9 texcoord register.
        RegisterMapping* texCoordMapping = m_map.lookupOrCreateRegisterMapping(
          *this,
          D3DSPR_TEXCRDOUT,
          samplerRegNum,
          ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE) | ENCODE_D3D10_SB_OPERAND_4_COMPONENT_MASK(D3D10_SB_OPERAND_4_COMPONENT_MASK_X | D3D10_SB_OPERAND_4_COMPONENT_MASK_Y),
          0,
          true
		  );

        texCoordOp = texCoordMapping->dxbcOperand;

        texCoordOp.setSwizzleOrWritemask(
          ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE) |
          ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE(D3D10_SB_4_COMPONENT_X, D3D10_SB_4_COMPONENT_Y, D3D10_SB_4_COMPONENT_X, D3D10_SB_4_COMPONENT_X)
        );
      }
      else {
        // SM1.4+
        DX9Operand texCoord{ lookupOperandInfo(optype::Src0), nextToken() };

        if (texCoord.getRegType() == D3DSPR_TEXTURE) {
          RegisterMapping* texCoordMapping = m_map.lookupOrCreateRegisterMapping(
            *this,
            D3DSPR_TEXCRDOUT,
            texCoord.getRegNumber(),
            calcReadMask(texCoord),
            0,
            true
          );

          texCoordOp = texCoordMapping->dxbcOperand;

          calculateDXBCSwizzleAndWriteMask(texCoordOp, texCoord);
          calculateDXBCModifiers(texCoordOp, operation, texCoord);
        }
        else
          texCoordOp = DXBCOperand{ *this, operation, texCoord, 0 };
      }

      if (getMajorVersion() >= 2) {
        DX9Operand sampler{ lookupOperandInfo(optype::Src1), nextToken() };
        samplerRegNum = sampler.getRegNumber();

        if (sampler.getRegType() != D3DSPR_SAMPLER)
          log::warn("Sampler for texld isn't a sampler");
      }

      if (!isSamplerUsed(samplerRegNum)) {
        log::warn("Adding an implicit 2D sampler.");

        SamplerDesc desc;
        desc.index = samplerRegNum;
        desc.dimension = D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D;

        m_samplers.push_back(desc);
      }

      // Why does this work?:
      // In SM1.1 - SM1.3 dst is a tx register, which we map to a temp, which is then moved later.
      // In SM1.4 the dst is a rx register, which we map to the right temp there.
      // In SM2.0+ the dst is also an rx, and we use a sampler different to the reg number.

      RegisterMapping* dstMapping = m_map.lookupOrCreateRegisterMapping(*this, dst);
      DXBCOperand dstOp = dstMapping->dxbcOperand;
      dstOp.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      calculateDXBCSwizzleAndWriteMask(dstOp, dst);
      calculateDXBCModifiers(dstOp, operation, dst);

      DXBCOperand textureOp{ D3D10_SB_OPERAND_TYPE_RESOURCE, 1 };
      textureOp.setData(&samplerRegNum, 1);
      textureOp.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      textureOp.setSwizzleOrWritemask(noSwizzle);

      DXBCOperand samplerOp{ D3D10_SB_OPERAND_TYPE_SAMPLER, 1 };
      samplerOp.setData(&samplerRegNum, 1);
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