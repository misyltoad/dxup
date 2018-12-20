#pragma once
#include "../util/fourcc.h"
#include "../util/misc_helpers.h"
#include "../util/config.h"
#include "../util/shared_conversions.h"
#include "dx9asm_translator.h"
#include "dxbc_header.h"
#include "dxbc_stats.h"
#include <string>

namespace dxup {

  namespace dx9asm {

    #pragma pack(1)
    struct ChunkHeader {
      ChunkHeader(uint32_t name) : name{ name }, size{ 0 } {}

      inline void push(std::vector<uint32_t>& obj) {
        obj.push_back(name);
        obj.push_back(size);
      }

      uint32_t name;
      uint32_t size;
    };

    void writeRDEF(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode);
    void writeSHEX(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode);
    void writeSTAT(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode);
    void writeISGN(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode);
    void writeOSGN(ShaderBytecode& bytecode, ShaderCodeTranslator& shdrCode);

  }

}