#include "dx9asm_translator.h"
#include "dx9asm_modifiers.h"
#include "../util/fourcc.h"
#include "../util/config.h"
#include <array>
#include <functional>

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

      mapping.dclInfo.usage = usageToken->getUsage();
      mapping.dclInfo.usageIndex = usageToken->getUsageIndex();

      if (dst->getRegType() == D3DSPR_INPUT) {
        mapping.dclInfo.type = UsageType::Input;

        if (getMajorVersion() != 3 && getShaderType() == ShaderType::Pixel)
          return true;
      }
      else if (dst->getRegType() == D3DSPR_TEXTURE) {
        mapping.dclInfo.type = UsageType::Input;
        mapping.dclInfo.usage = D3DDECLUSAGE_TEXCOORD;
        mapping.dclInfo.usageIndex = dst->getRegNumber();
      }
      else if (dst->getRegType() == D3DSPR_OUTPUT) {
        mapping.dxbcOperand.setRegisterType(D3D10_SB_OPERAND_TYPE_OUTPUT);
        mapping.dclInfo.type = UsageType::Output;
      }
      else if (dst->getRegType() == D3DSPR_SAMPLER) {

        SamplerDesc desc;
        desc.index = dst->getRegNumber();
        switch (usageToken->getTextureType())
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

      if (getShaderType() == ShaderType::Pixel && mapping.dclInfo.type == UsageType::Output) {
        if (mapping.dclInfo.usage == D3DDECLUSAGE_COLOR)
          mapping.dclInfo.target = true;
        else if (mapping.dclInfo.usage == D3DDECLUSAGE_DEPTH)
          log::fail("Writing to oDepth not supported yet.");
      }

      mapping.dclInfo.centroid = dst->centroid();

      // This may get changed later...
      mapping.dxbcOperand.setData(&mapping.dx9Id, 1);

      bool io = mapping.dclInfo.type != UsageType::None;
      bool transient = io && isTransient(mapping.dclInfo.type == UsageType::Input);
      bool generateId = shouldGenerateId(transient);

      getRegisterMap().addRegisterMapping(transient, generateId, mapping);

      return true;
    }

    bool ShaderCodeTranslator::handleMov(DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);

      if (src0->getRegType() != D3DSPR_CONSTINT && dst->getRegType() == D3DSPR_ADDR && getMajorVersion() == 1 && getMinorVersion() == 1)
        return handleMova(operation);

      DXBCOperand dstOp = { *this, operation, *dst, 0 };
      DXBCOperand srcOp = { *this, operation, *src0, 0 };

      DXBCOperation{ D3D10_SB_OPCODE_MOV, operation.saturate() }
        .appendOperand(dstOp)
        .appendOperand(srcOp)
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handleMova(DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);

      if (src0->getRegType() == D3DSPR_CONSTINT)
        return handleMov(operation);

      DXBCOperand dstOp = { *this, operation, *dst, 0 };
      DXBCOperand srcOp = { *this, operation, *src0, 0 };

      DXBCOperand tempOpSrc = getRegisterMap().getNextInternalTemp();
      DXBCOperand tempOpDst = tempOpSrc;
      tempOpSrc.setSwizzleOrWritemask(noSwizzle);
      tempOpDst.setSwizzleOrWritemask(writeAll);

      // dst = round(src)
      // Human rounding, dst is an addr register which is mapped to a temp for us.
      DXBCOperation{ D3D10_SB_OPCODE_ROUND_NI, false }
        .appendOperand(tempOpDst)
        .appendOperand(srcOp)
        .push(*this);

      // dst = int(dst)
      DXBCOperation{ D3D10_SB_OPCODE_FTOI, false }
        .appendOperand(dstOp)
        .appendOperand(tempOpSrc)
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handleScomp(bool lt, DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);
      const DX9Operand* src1 = operation.getOperandByType(optype::Src1);

      DXBCOperand dstOp = { *this, operation, *dst, 0 };
      DXBCOperand src0Op = { *this, operation, *src0, 0 };
      DXBCOperand src1Op = { *this, operation, *src1, 0 };

      DXBCOperand tempOpSrc = getRegisterMap().getNextInternalTemp();
      DXBCOperand tempOpDst = tempOpSrc;
      tempOpSrc.setSwizzleOrWritemask(noSwizzle);
      tempOpDst.setSwizzleOrWritemask(writeAll);

      DXBCOperation{ (uint32_t)(lt ? D3D10_SB_OPCODE_LT : D3D10_SB_OPCODE_GE), false }
        .appendOperand(tempOpDst)
        .appendOperand(src0Op)
        .appendOperand(src1Op)
        .push(*this);

      // Bithacking for uint32_t 0xFFFFFFFF -> float 1.0f. uint32_t 0x00000000 -> float 0.0f.
      const uint32_t andFixup = 0x3f800000;
      DXBCOperation{ D3D10_SB_OPCODE_AND, operation.saturate() }
        .appendOperand(dstOp)
        .appendOperand(tempOpSrc)
        .appendOperand(DXBCOperand{ andFixup, andFixup, andFixup, andFixup })
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handleSlt(DX9Operation& operation) {
      return handleScomp(true, operation);
    }
    bool ShaderCodeTranslator::handleSge(DX9Operation& operation) {
      return handleScomp(false, operation);
    }

    bool ShaderCodeTranslator::handleCmp(DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);
      const DX9Operand* src1 = operation.getOperandByType(optype::Src1);
      const DX9Operand* src2 = operation.getOperandByType(optype::Src2);

      DXBCOperand dstOp = { *this, operation, *dst, 0 };
      DXBCOperand src0Op = { *this, operation, *src0, 0 };
      DXBCOperand src1Op = { *this, operation, *src1, 0 };
      DXBCOperand src2Op = { *this, operation, *src2, 0 };

      DXBCOperand tempOpSrc = getRegisterMap().getNextInternalTemp();
      DXBCOperand tempOpDst = tempOpSrc;
      tempOpSrc.setSwizzleOrWritemask(noSwizzle);
      tempOpDst.setSwizzleOrWritemask(writeAll);

      DXBCOperation{ D3D10_SB_OPCODE_GE, false }
        .appendOperand(tempOpDst)
        .appendOperand(src0Op)
        .appendOperand({ 0, 0, 0, 0 })
        .push(*this);

      DXBCOperation{ D3D10_SB_OPCODE_MOVC, operation.saturate() }
        .appendOperand(dstOp)
        .appendOperand(tempOpSrc)
        .appendOperand(src1Op)
        .appendOperand(src2Op)
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handleNrm(DX9Operation& operation) {
      DX9Operand* dst = operation.getOperandByType(optype::Dst);
      DX9Operand* src0 = operation.getOperandByType(optype::Src0);

      DXBCOperand srcOp = { *this, operation, *src0, 0 };
      DXBCOperand dstOp = { *this, operation, *dst, 0 };

      DXBCOperand tempOpDst = getRegisterMap().getNextInternalTemp();
      DXBCOperand tempOpSrc = tempOpDst;
      tempOpDst.setSwizzleOrWritemask(writeAll);
      tempOpSrc.setSwizzleOrWritemask(noSwizzle);

      // DP with self to get length squared.
      // This is DP3 because nrm only applies for 3D vectors...
      DXBCOperation{ D3D10_SB_OPCODE_DP3, false }
        .appendOperand(tempOpDst)
        .appendOperand(srcOp)
        .appendOperand(srcOp)
        .push(*this);

      DXBCOperation{ D3D10_SB_OPCODE_RSQ, false }
        .appendOperand(tempOpDst)
        .appendOperand(tempOpSrc)
        .push(*this);

      DXBCOperation{ D3D10_SB_OPCODE_MUL, operation.saturate() }
        .appendOperand(dstOp)
        .appendOperand(srcOp)
        .appendOperand(tempOpSrc)
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handlePow(DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);
      const DX9Operand* src1 = operation.getOperandByType(optype::Src1);

      DXBCOperand dstOp = { *this, operation, *dst, 0 };
      DXBCOperand src0Op = { *this, operation, *src0, 0 };
      DXBCOperand src1Op = { *this, operation, *src1, 0 };

      DXBCOperand tempOpDst = getRegisterMap().getNextInternalTemp();
      DXBCOperand tempOpSrc = tempOpDst;
      tempOpDst.setSwizzleOrWritemask(writeAll);
      tempOpSrc.setSwizzleOrWritemask(noSwizzle);

      DXBCOperation{ D3D10_SB_OPCODE_LOG, false }
        .appendOperand(tempOpDst)
        .appendOperand(src0Op)
        .push(*this);

      DXBCOperation{ D3D10_SB_OPCODE_MUL, false }
        .appendOperand(tempOpDst)
        .appendOperand(tempOpSrc)
        .appendOperand(src1Op)
        .push(*this);

      DXBCOperation{ D3D10_SB_OPCODE_EXP, operation.saturate() }
        .appendOperand(dstOp)
        .appendOperand(tempOpSrc)
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
        texCoord.setUsedComponents(2);

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

      DXBCOperation{D3D10_SB_OPCODE_SAMPLE, operation.saturate()}
        .setSampler(true)
        .appendOperand(dstOp)
        .appendOperand(texCoordOp)
        .appendOperand(textureOp)
        .appendOperand(samplerOp)
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handleGenericTexReg2XX(DX9Operation& operation, uint32_t swizzle) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);

      DXBCOperand dstOp = { *this, operation, *dst, 0 };
      DXBCOperand srcOp = { *this, operation, *src0, 0 };

      uint32_t samplerRegNum = dstOp.getRegNumber();

      DXBCOperand texCoordOp = srcOp;
      texCoordOp.setSwizzleOrWritemask(swizzle);

      DXBCOperand textureOp{ D3D10_SB_OPERAND_TYPE_RESOURCE, 1 };
      textureOp.setData(&samplerRegNum, 1);
      textureOp.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
      textureOp.setSwizzleOrWritemask(noSwizzle);

      DXBCOperand samplerOp{ D3D10_SB_OPERAND_TYPE_SAMPLER, 1 };
      samplerOp.setData(&samplerRegNum, 1);
      samplerOp.setComponents(0);
      samplerOp.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);

      if (!isSamplerUsed(samplerRegNum)) {
        log::warn("Adding an implicit 2D sampler.");

        SamplerDesc desc;
        desc.index = samplerRegNum;
        desc.dimension = D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D;

        m_samplers.push_back(desc);
      }

      DXBCOperation{ D3D10_SB_OPCODE_SAMPLE, operation.saturate() }
        .setSampler(true)
        .appendOperand(dstOp)
        .appendOperand(texCoordOp)
        .appendOperand(textureOp)
        .appendOperand(samplerOp)
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handleTexReg2Ar(DX9Operation& operation) {
      return handleGenericTexReg2XX(operation, ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE) |
        ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE(D3D10_SB_4_COMPONENT_W, D3D10_SB_4_COMPONENT_R, D3D10_SB_4_COMPONENT_R, D3D10_SB_4_COMPONENT_R));
    }

    bool ShaderCodeTranslator::handleTexReg2Gb(DX9Operation& operation) {
      return handleGenericTexReg2XX(operation, ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE) |
        ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE(D3D10_SB_4_COMPONENT_G, D3D10_SB_4_COMPONENT_B, D3D10_SB_4_COMPONENT_B, D3D10_SB_4_COMPONENT_B));
    }

    bool ShaderCodeTranslator::handleLrp(DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);
      const DX9Operand* src1 = operation.getOperandByType(optype::Src1);
      const DX9Operand* src2 = operation.getOperandByType(optype::Src2);

      DXBCOperand dstOp = { *this, operation, *dst, 0 };
      DXBCOperand src0Op = { *this, operation, *src0, 0 };
      DXBCOperand src1Op = { *this, operation, *src1, 0 };
      DXBCOperand src2Op = { *this, operation, *src2, 0 };
      DXBCOperand src2OpNeg = src2Op;
      src2OpNeg.setModifier(src2OpNeg.getModifier() ^ D3D10_SB_OPERAND_MODIFIER_NEG);

      DXBCOperand tempOpSrc = getRegisterMap().getNextInternalTemp();
      DXBCOperand tempOpDst = tempOpSrc;
      tempOpSrc.setSwizzleOrWritemask(noSwizzle);
      tempOpDst.setSwizzleOrWritemask(writeAll);

      // (src1 - src2)
      DXBCOperation{ D3D10_SB_OPCODE_ADD, false }
        .appendOperand(tempOpDst)
        .appendOperand(src1Op)
        .appendOperand(src2OpNeg)
        .push(*this);

      // src2 + src0 * (src1 - src2)
      DXBCOperation{ D3D10_SB_OPCODE_MAD, operation.saturate() }
        .appendOperand(dstOp)
        .appendOperand(src0Op)
        .appendOperand(tempOpSrc)
        .appendOperand(src2Op)
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handleGenericDef(DX9Operation& operation, bool boolean) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* vec4 = operation.getOperandByType(boolean ? optype::Bool : optype::Vec4);

      RegisterMapping mapping;
      mapping.dx9Id = dst->getRegNumber();
      mapping.dx9Type = dst->getRegType();
      uint32_t data[4];
      vec4->getValues(data);
      if (boolean) {
        data[1]  = data[0];
        data[2]  = data[0];
        data[3]  = data[0];
      }
      mapping.dxbcOperand.setData(data, 4);
      mapping.dxbcOperand.setupLiteral(4);

      mapping.dxbcOperand.setSwizzleOrWritemask(noSwizzle);

      m_map.addRegisterMapping(false, false, mapping);

      return true;
    }

    bool ShaderCodeTranslator::handleDef(DX9Operation& operation) {
      return handleGenericDef(operation, false);
    }

    bool ShaderCodeTranslator::handleDefi(DX9Operation& operation) {
      return handleGenericDef(operation, false);
    }

    bool ShaderCodeTranslator::handleDefB(DX9Operation& operation) {
      return handleGenericDef(operation, true);
    }

    bool ShaderCodeTranslator::handleSinCos(DX9Operation& operation) {
      DX9Operand dst{ lookupOperandInfo(optype::Dst), nextToken() };
      DX9Operand src0{ lookupOperandInfo(optype::Src0), nextToken() };

      if (getMajorVersion() < 3) {
        DX9Operand temp1{ lookupOperandInfo(optype::Src1), nextToken() };
        DX9Operand temp2{ lookupOperandInfo(optype::Src2), nextToken() };
        // We don't use these, but they need to be parsed away.
      }

      DXBCOperand sinDstOp = { *this, operation, dst, 0 };
      DXBCOperand cosDstOp = { *this, operation, dst, 0 };
      DXBCOperand src0Op = { *this, operation, src0, 0 };

      uint32_t sinMask = 0;
      if (dst.getWriteMaskData() & D3DSP_WRITEMASK_0)
        sinMask = D3DSP_WRITEMASK_0;
      else if (dst.getWriteMaskData() & D3DSP_WRITEMASK_1)
        sinMask = D3DSP_WRITEMASK_1;

      uint32_t cosMask = 0;

      if (dst.getWriteMaskData() & D3DSP_WRITEMASK_0 && dst.getWriteMaskData() & D3DSP_WRITEMASK_1)
        cosMask = D3DSP_WRITEMASK_1;

      if (sinMask)
        sinDstOp.setSwizzleOrWritemask(calcWriteMask(sinMask));
      else {
        sinDstOp.setRegisterType(D3D10_SB_OPERAND_TYPE_NULL);
        sinDstOp.setDimension(D3D10_SB_OPERAND_INDEX_0D);
        sinDstOp.setData(nullptr, 0);
        sinDstOp.setComponents(0);
        sinDstOp.stripModifier();
      }

      if (cosMask)
        cosDstOp.setSwizzleOrWritemask(calcWriteMask(cosMask));
      else {
        cosDstOp.setRegisterType(D3D10_SB_OPERAND_TYPE_NULL);
        cosDstOp.setData(nullptr, 0);
        cosDstOp.setDimension(D3D10_SB_OPERAND_INDEX_0D);
        cosDstOp.setComponents(0);
        cosDstOp.stripModifier();
      }

      DXBCOperation{ D3D10_SB_OPCODE_SINCOS, operation.saturate() }
        .appendOperand(sinDstOp)
        .appendOperand(cosDstOp)
        .appendOperand(src0Op)
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handleDp2Add(DX9Operation& operation) {
      const DX9Operand* dst = operation.getOperandByType(optype::Dst);
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);
      const DX9Operand* src1 = operation.getOperandByType(optype::Src1);
      const DX9Operand* src2 = operation.getOperandByType(optype::Src2);

      DXBCOperand dstOp = { *this, operation, *dst, 0 };
      DXBCOperand src0Op = { *this, operation, *src0, 0 };
      DXBCOperand src1Op = { *this, operation, *src1, 0 };
      DXBCOperand src2Op = { *this, operation, *src2, 0 };

      DXBCOperand tempOpSrc = getRegisterMap().getNextInternalTemp();
      DXBCOperand tempOpDst = tempOpSrc;
      tempOpSrc.setComponents(noSwizzle);
      tempOpDst.setSwizzleOrWritemask(writeAll);

      DXBCOperation{ D3D10_SB_OPCODE_DP2, false }
        .appendOperand(tempOpDst)
        .appendOperand(src0Op)
        .appendOperand(src1Op)
        .push(*this);

      DXBCOperation{ D3D10_SB_OPCODE_ADD, operation.saturate() }
        .appendOperand(dstOp)
        .appendOperand(tempOpSrc)
        .appendOperand(src2Op)
        .push(*this);

      return true;
    }

    struct CompareMode {
      uint32_t opcode;
      bool swap;
    };

    // Should match DXBC code gen.
    std::array<CompareMode, 7> compareModes = {
      CompareMode{D3D10_SB_OPCODE_ADD, false}, // Invalid
      CompareMode{D3D10_SB_OPCODE_LT, true}, // >
      CompareMode{D3D10_SB_OPCODE_EQ, false}, // ==
      CompareMode{D3D10_SB_OPCODE_GE, false}, // >=
      CompareMode{D3D10_SB_OPCODE_LT, false}, // <
      CompareMode{D3D10_SB_OPCODE_NE, false}, // !=
      CompareMode{D3D10_SB_OPCODE_GE, true} // <=
    };

    bool ShaderCodeTranslator::handleIfc(DX9Operation& operation) {
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);
      const DX9Operand* src1 = operation.getOperandByType(optype::Src1);

      DXBCOperand src0Op = { *this, operation, *src0, 0 };
      DXBCOperand src1Op = { *this, operation, *src1, 0 };

      uint32_t compare = operation.getOpcodeSpecificData();
      if (compare < 1 || compare > 6) {
        log::fail("Invalid comparemode in ifc.");
        return false;
      }

      CompareMode compareMode = compareModes[compare];

      DXBCOperand tempOpSrc = getRegisterMap().getNextInternalTemp();
      DXBCOperand tempOpDst = tempOpSrc;
      tempOpSrc.setComponents(ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE(D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE) | ENCODE_D3D10_SB_OPERAND_4_COMPONENT_SELECT_1(D3D10_SB_4_COMPONENT_X));
      tempOpDst.setSwizzleOrWritemask(writeAll);

      DXBCOperation{ compareMode.opcode, false }
        .appendOperand(tempOpDst)
        .appendOperand(compareMode.swap ? src1Op : src0Op)
        .appendOperand(compareMode.swap ? src0Op : src1Op)
        .push(*this);

      DXBCOperation{ D3D10_SB_OPCODE_IF, false }
        .setExtra(ENCODE_D3D10_SB_INSTRUCTION_TEST_BOOLEAN(D3D10_SB_INSTRUCTION_TEST_NONZERO))
        .appendOperand(tempOpSrc)
        .push(*this);

      return true;
    }

    bool ShaderCodeTranslator::handleIf(DX9Operation& operation) {
      const DX9Operand* src0 = operation.getOperandByType(optype::Src0);
      DXBCOperand src0Op = { *this, operation, *src0, 0 };

      DXBCOperation{ D3D10_SB_OPCODE_IF, false }
        .setExtra(ENCODE_D3D10_SB_INSTRUCTION_TEST_BOOLEAN(D3D10_SB_INSTRUCTION_TEST_NONZERO))
        .appendOperand(src0Op)
        .push(*this);

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
