#include "dx9asm_operations.h"
#include "dx9asm_util.h"
#include "dx9asm_translator.h"

namespace dxup {

  namespace dx9asm {

    namespace {
      using namespace optype;
      using namespace implicitflag;

      std::vector<DX9OperationInfo> operationInfos = {
        {"abs",     D3DSIO_ABS, 1, { Dst, Src0 }, { D3D10_SB_OPCODE_MOV, abs} },
        {"add",     D3DSIO_ADD, 1, {Dst, Src0, Src1}, { D3D10_SB_OPCODE_ADD, 0} },
        {"bem",     D3DSIO_BEM, 1, {Dst, Src0, Src1}, {} },
        {"break",   D3DSIO_BREAK, 1, {}, {}},
        {"breakc",  D3DSIO_BREAKC, 1, {}, {}},
        {"breakp",  D3DSIO_BREAKP, 1, {}, {}},
        {"call",    D3DSIO_CALL, 1, {Label}, {}},
        {"callnz",  D3DSIO_CALLNZ, 1, {Label, Src0}, {}},
        {"cmp",     D3DSIO_CMP, 1, {Dst, Src0, Src1, Src2}, {}, &ShaderCodeTranslator::handleCmp},
        {"cnd",     D3DSIO_CND, 1, {Dst, Src0, Src1, Src2}, {} },
        {"crs",     D3DSIO_CRS, 1, {Dst, Src0, Src1}, {}},
        {"comment", D3DSIO_COMMENT, 1, {VaradicOperandCount}, {}, &ShaderCodeTranslator::handleComment},
        {"dcl",     D3DSIO_DCL, 1, {UsageToken, Dst}, {}, &ShaderCodeTranslator::handleDcl},
        {"def",     D3DSIO_DEF, 1, {Dst, Vec4}, {}, &ShaderCodeTranslator::handleDef},
        {"defb",    D3DSIO_DEFB, 1, {Dst, Bool}, {}, &ShaderCodeTranslator::handleDefB},
        {"defi",    D3DSIO_DEFI, 1, {Dst, Vec4}, {}, &ShaderCodeTranslator::handleDefi},
        {"dp2add",  D3DSIO_DP2ADD, 1, {Dst, Src0, Src1, Src2}, {}, &ShaderCodeTranslator::handleDp2Add},
        {"dp3",     D3DSIO_DP3, 1, {Dst, Src0, Src1}, { D3D10_SB_OPCODE_DP3, 0, 3, 3 }},
        {"dp4",     D3DSIO_DP4, 1, {Dst, Src0, Src1}, { D3D10_SB_OPCODE_DP4, 0 }},
        {"dsx",     D3DSIO_DSX, 1, {Dst, Src0}, {}},
        {"dsy",     D3DSIO_DSY, 1, {Dst, Src0}, {}},
        {"dst",     D3DSIO_DST, 1, {Dst, Src0, Src1}, {}},
        {"else",    D3DSIO_ELSE, 1, {}, {D3D10_SB_OPCODE_ELSE, 0}},
        {"endif",   D3DSIO_ENDIF, 1, {}, {D3D10_SB_OPCODE_ENDIF, 0}},
        {"endloop", D3DSIO_ENDLOOP, 1, {}, {}},
        {"endrep",  D3DSIO_ENDREP, 1, {}, {}},
        {"exp",     D3DSIO_EXP, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_EXP, 0}},
        {"expp",    D3DSIO_EXPP, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_EXP, 0}},
        {"frc",     D3DSIO_FRC, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_FRC, 0}},
        {"if",      D3DSIO_IF, 1, { Src0 }, {}, &ShaderCodeTranslator::handleIf},
        {"ifc",     D3DSIO_IFC, 1, { Src0, Src1 }, {}, &ShaderCodeTranslator::handleIfc},
        {"label",   D3DSIO_LABEL, 1, { Label }, {}},
        {"lit",     D3DSIO_LIT, 1, { Dst, Src0 }, {}},
        {"log",     D3DSIO_LOG, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_LOG, 0}},
        {"logp",    D3DSIO_LOGP, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_LOG, 0}},
        {"loop",    D3DSIO_LOOP, 1, { LoopCounter, Src0 }, {}},
        {"lrp",     D3DSIO_LRP, 1, { Dst, Src0, Src1, Src2 }, {}, &ShaderCodeTranslator::handleLrp},
        {"m3x2",    D3DSIO_M3x2, 2, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_DP3, 0, 3, 3}},
        {"m3x3",    D3DSIO_M3x3, 3, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_DP3, 0, 3, 3}},
        {"m3x4",    D3DSIO_M3x4, 4, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_DP3, 0, 3, 3}},
        {"m4x3",    D3DSIO_M4x3, 3, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_DP4, 0, 4, 4}},
        {"m4x4",    D3DSIO_M4x4, 4, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_DP4, 0, 4, 4}},
        {"mad",     D3DSIO_MAD, 1, { Dst, Src0, Src1, Src2 }, {D3D10_SB_OPCODE_MAD, 0}},
        {"max",     D3DSIO_MAX, 1, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_MAX, 0}},
        {"min",     D3DSIO_MIN, 1, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_MIN, 0}},
        {"mov",     D3DSIO_MOV, 1, { Dst, Src0 }, {}, &ShaderCodeTranslator::handleMov},
        {"mova",    D3DSIO_MOVA, 1, { Dst, Src0 }, {}, &ShaderCodeTranslator::handleMova},
        {"mul",     D3DSIO_MUL, 1, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_MUL, 0}},
        {"nop",     D3DSIO_NOP, 1, { }, {D3D10_SB_OPCODE_NOP, 0}},
        {"nrm",     D3DSIO_NRM, 1, { Dst, Src0 }, {}, &ShaderCodeTranslator::handleNrm},
        {"phase",   D3DSIO_PHASE, 1, {}, {}},
        {"pow",     D3DSIO_POW, 1, { Dst, Src0, Src1 }, {}, &ShaderCodeTranslator::handlePow},
        {"rcp",     D3DSIO_RCP, 1, { Dst, Src0 }, {D3D11_SB_OPCODE_RCP, 0}},
        {"rep",     D3DSIO_REP, 1, { Src0 }, {}},
        {"ret",     D3DSIO_RET, 1, {}, {}},
        {"rsq",     D3DSIO_RSQ, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_RSQ, 0}},
        {"setp",    D3DSIO_SETP, 1, { Dst, Src0, Src1 }, {}},
        {"sge",     D3DSIO_SGE, 1, { Dst, Src0, Src1 }, {}, &ShaderCodeTranslator::handleSge},
        {"sgn",     D3DSIO_SGN, 1, { Dst, Src0, Src1, Src2 }, {}},
        {"sincos",  D3DSIO_SINCOS, 1, { VaradicOperandCount }, {}, &ShaderCodeTranslator::handleSinCos},
        {"slt",     D3DSIO_SLT, 1, { Dst, Src0, Src1 }, {}, &ShaderCodeTranslator::handleSlt},
        {"sub",     D3DSIO_SUB, 1, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_ADD, negate}},

        {"tex",     D3DSIO_TEX, 1, { VaradicOperandCount }, {}, &ShaderCodeTranslator::handleTex}, // The destination register number specifies the texture stage number.
        {"texbem",  D3DSIO_TEXBEM, 1, { Dst, Src0 }, {}},
        {"texbeml", D3DSIO_TEXBEML, 1, { Dst, Src0 }, {}},
        {"texcoord",D3DSIO_TEXCOORD, 1, { Dst }, {}},
        //{"texcrd",  D3DSIO_TEXCRD, 1, { Dst, Src0 }, {}},
        {"texdepth",D3DSIO_TEXDEPTH, 1, { Dst }, {}},
        {"texdp3",  D3DSIO_TEXDP3, 1, { Dst, Src0 }, {}},
        {"texdp3tex",  D3DSIO_TEXDP3TEX, 1, { Dst, Src0 }, {}},
        {"texkill",  D3DSIO_TEXKILL, 1, { Dst }, {}},
        //{"texldb",  D3DSIO_TEXLDB, 1, { Dst, Src0, Src1 }, {}},
        {"texldd",  D3DSIO_TEXLDD, 1, { Dst, Src0, Src1, Src2, Src3 }, {}},
        {"texldl",  D3DSIO_TEXLDL, 1, { Dst, Src0, Src1 }, {}},
        //{"texldp",  D3DSIO_TEXLDP, 1, { Dst, Src0, Src1 }, {}},
        {"texm3x2depth",  D3DSIO_TEXM3x2DEPTH, 1, { Dst, Src0 }, {}},
        {"texm3x2pad",  D3DSIO_TEXM3x2PAD, 1, { Dst, Src0 }, {}},
        {"texm3x2tex",  D3DSIO_TEXM3x2TEX, 1, { Dst, Src0 }, {}},
        {"texm3x3",  D3DSIO_TEXM3x3, 1, { Dst, Src0 }, {}},
        {"texm3x3pad",  D3DSIO_TEXM3x3PAD, 1, { Dst, Src0 }, {}},
        {"texm3x3spec",  D3DSIO_TEXM3x3SPEC, 1, { Dst, Src0, Src1 }, {}},
        {"texm3x3tex",  D3DSIO_TEXM3x3TEX, 1, { Dst, Src0 }, {}},
        {"texm3x3vspec",  D3DSIO_TEXM3x3VSPEC, 1, { Dst, Src0 }, {}},
        {"texreg2ar",  D3DSIO_TEXREG2AR, 1, { Dst, Src0 }, {}, &ShaderCodeTranslator::handleTexReg2Ar},
        {"texreg2gb",  D3DSIO_TEXREG2GB, 1, { Dst, Src0 }, {}, &ShaderCodeTranslator::handleTexReg2Gb},
        {"texreg2rgb",  D3DSIO_TEXREG2RGB, 1, { Dst, Src0 }, {}},
      };
    }

    const DX9OperationInfo* lookupOperationInfo(uint32_t token) {
      for (size_t i = 0; i < operationInfos.size(); i++) {
        DX9OperationInfo* info = &operationInfos[i];
        if (info->dx9opcode == opcode(token))
          return info;
      }
      return nullptr;
    }

    void DX9Operation::readOperands(ShaderCodeTranslator& translator) {
      for (OperandType arg : getArgs()) {
        const OperandInfo* info = lookupOperandInfo(arg);

        uint32_t tokens[4];
        for (uint32_t i = 0; i < info->sizeInTokens; i++)
          tokens[i] = translator.nextToken();

        m_operands.emplace_back(translator, info, tokens);
      }
    }

  }
}
