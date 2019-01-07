#include "dx9asm_translator.h"
#include "dx9asm_operations.h"
#include "dx9asm_modifiers.h"
#include "../util/config.h"
#include "../util/misc_helpers.h"
#include "dxbc_bytecode.h"

namespace dxup {

  namespace dx9asm {

    bool ShaderCodeTranslator::handleOperation(uint32_t token) {
      DX9Operation operation{ *this, token };
      if (!operation.isValid()) {
        log::fail("Unknown opcode %lu encountered!", opcode(token));
        return false;
      }

      if (config::getBool(config::ShaderSpew))
        log::msg("Translating operation %s.", operation.getName());

      if (operation.isUnique())
        return handleUniqueOperation(operation);
      else
        return handleStandardOperation(operation);

      return true;
    }

    bool ShaderCodeTranslator::translate() {
      nextToken(); // Skip header.

      uint32_t token = nextToken();
      while (token != D3DPS_END()) {
        if (!handleOperation(token))
          return false;
        token = nextToken();
      }

      // r0 in SM1 is the colour output register. Move r0 -> target here.
      if (getShaderType() == ShaderType::Pixel && getMajorVersion() == 1) {
        RegisterMapping* r0 = getRegisterMap().getRegisterMapping(D3DSPR_TEMP, 0);
        DXBCOperand r0Op = r0->dxbcOperand;
        r0Op.setSwizzleOrWritemask(noSwizzle);

        if (r0 != nullptr) {
          RegisterMapping* target = getRegisterMap().lookupOrCreateRegisterMapping(
            *this,
            D3DSPR_COLOROUT,
            0,
            0,
            writeAll
          );

          DXBCOperand targetOp = target->dxbcOperand;
          targetOp.setSwizzleOrWritemask(writeAll);

          DXBCOperation{ D3D10_SB_OPCODE_MOV, false }
          .appendOperand(targetOp).appendOperand(r0Op).push(*this);
        }
      }

      DXBCOperation{ D3D10_SB_OPCODE_RET, false }.push(*this);

      return true;
    }
  
    void toDXBC(const uint32_t* dx9asm, ShaderBytecode** dxbc) {
      InitReturnPtr(dxbc);

      if (dxbc == nullptr) {
        log::fail("toDXBC called with null ShaderBytecode to return.");
        return;
      }

      static ShaderCodeTranslator translator;
      translator.reset(dx9asm);

      if (!translator.translate()) {
        log::fail("Failed to translate shader fatally!");
        return;
      }

      *dxbc = new ShaderBytecode{translator};
    }

  }

}