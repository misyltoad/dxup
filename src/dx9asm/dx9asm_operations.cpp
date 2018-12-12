#include "dx9asm_operations.h"
#include "dx9asm_util.h"
#include "dx9asm_translator.h"

namespace dxapex {

  namespace dx9asm {

    namespace {
      using namespace optype;
      using namespace implicitflag;

      std::vector<DX9OperationInfo> operationInfos = {
        {"abs",     D3DSIO_ABS, 1, { Dst, Src0 }, { D3D10_SB_OPCODE_MOV, abs} },
        {"add",     D3DSIO_ADD, 1, {Dst, Src0, Src1}, { D3D10_SB_OPCODE_ADD, 0} },
        {"break",   D3DSIO_BREAK, 1, {}, {}},
        {"breakc",  D3DSIO_BREAKC, 1, {}, {}},
        {"breakp",  D3DSIO_BREAKP, 1, {}, {}},
        {"call",    D3DSIO_CALL, 1, {Label}, {}},
        {"callnz",  D3DSIO_CALLNZ, 1, {Label, Src0}, {}},
        {"crs",     D3DSIO_CRS, 1, {Dst, Src0, Src1}, {}},
        {"comment", D3DSIO_COMMENT, 1, {VaradicOperandCount}, {}, &ShaderCodeTranslator::handleComment},
        {"dcl",     D3DSIO_DCL, 1, {UsageToken, Dst}, {}, &ShaderCodeTranslator::handleDcl},
        {"def",     D3DSIO_DEF, 1, {Dst, Vec4}, {}, &ShaderCodeTranslator::handleDef},
        {"defb",    D3DSIO_DEFB, 1, {Dst, Bool}, {}},
        {"defi",    D3DSIO_DEFI, 1, {Dst, Integer}, {}},
        {"dp3",     D3DSIO_DP3, 1, {Dst, Src0, Src1}, { D3D10_SB_OPCODE_DP3, 0 }},
        {"dp4",     D3DSIO_DP4, 1, {Dst, Src0, Src1}, { D3D10_SB_OPCODE_DP4, 0 }},
        {"dst",     D3DSIO_DST, 1, {Dst, Src0, Src1}, {}},
        {"else",    D3DSIO_ELSE, 1, {}, {}},
        {"endif",   D3DSIO_ENDIF, 1, {}, {}},
        {"endloop", D3DSIO_ENDLOOP, 1, {}, {}},
        {"endrep",  D3DSIO_ENDREP, 1, {}, {}},
        {"exp",     D3DSIO_EXP, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_EXP, 0}},
        {"expp",    D3DSIO_EXPP, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_EXP, 0}},
        {"frc",     D3DSIO_FRC, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_FRC, 0}},
        {"if",      D3DSIO_IF, 1, { Src0 }, {}},
        {"ifc",     D3DSIO_IFC, 1, { Src0, Src1 }, {}},
        {"ifc",     D3DSIO_IFC, 1, { Src0, Src1 }, {}},
        {"label",   D3DSIO_LABEL, 1, { Label }, {}},
        {"lit",     D3DSIO_LIT, 1, { Dst, Src0 }, {}},
        {"log",     D3DSIO_LOG, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_LOG, 0}},
        {"logp",    D3DSIO_LOGP, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_LOG, 0}},
        {"loop",    D3DSIO_LOOP, 1, { LoopCounter, Src0 }, {}},
        {"lrp",     D3DSIO_LRP, 1, { Dst, Src0, Src1, Src2 }, {}},
        {"m3x2",    D3DSIO_M3x2, 2, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_DP3, 0}},
        {"m3x3",    D3DSIO_M3x3, 3, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_DP3, 0}},
        {"m3x4",    D3DSIO_M3x4, 4, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_DP3, 0}},
        {"m4x3",    D3DSIO_M4x3, 3, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_DP4, 0}},
        {"m4x3",    D3DSIO_M4x3, 4, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_DP4, 0}},
        {"mad",     D3DSIO_MAD, 1, { Dst, Src0, Src1, Src2 }, {D3D10_SB_OPCODE_MAD, 0}},
        {"max",     D3DSIO_MAX, 1, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_MAX, 0}},
        {"min",     D3DSIO_MIN, 1, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_MIN, 0}},
        {"mov",     D3DSIO_MOV, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_MOV, 0}},
        {"mova",    D3DSIO_MOVA, 1, { Dst, Src0 }, {}},
        {"mul",     D3DSIO_MUL, 1, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_MUL, 0}},
        {"nop",     D3DSIO_NOP, 1, { }, {D3D10_SB_OPCODE_NOP, 0}},
        {"nrm",     D3DSIO_NRM, 1, { Dst, Src0, Src1 }, {}},
        {"pow",     D3DSIO_POW, 1, { Dst, Src0, Src1 }, {}},
        {"rcp",     D3DSIO_RCP, 1, { Dst, Src0 }, {D3D11_SB_OPCODE_RCP, 0}},
        {"rep",     D3DSIO_REP, 1, { Src0 }, {}},
        {"ret",     D3DSIO_RET, 1, {}, {}},
        {"rsq",     D3DSIO_RSQ, 1, { Dst, Src0 }, {D3D10_SB_OPCODE_RSQ, 0}},
        {"setp",    D3DSIO_SETP, 1, { Dst, Src0, Src1 }, {}},
        {"sge",     D3DSIO_SGE, 1, { Dst, Src0, Src1 }, {}},
        {"sgn",     D3DSIO_SGN, 1, { Dst, Src0, Src1, Src2 }, {}},
        {"sincos",  D3DSIO_SINCOS, 1, { Dst, Src0, Src1, Src2 }, {}},
        {"slt",     D3DSIO_SLT, 1, { Dst, Src0, Src1 }, {}},
        {"sub",     D3DSIO_SUB, 1, { Dst, Src0, Src1 }, {D3D10_SB_OPCODE_ADD, negate}},
        {"tex",     D3DSIO_TEX, 1, { Dst }, {}}, // The destination register number specifies the texture stage number.
        {"texldl",  D3DSIO_TEXLDL, 1, { Dst, Src0, Src1 }, {}},
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

        m_operands.emplace_back(info, tokens);
      }
    }

  }
}