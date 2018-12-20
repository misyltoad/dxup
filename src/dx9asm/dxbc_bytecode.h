#pragma once

#include "dxbc_header.h"
#include <vector>

namespace dxup {

  namespace dx9asm {

    class ShaderCodeTranslator;

    class ShaderBytecode {
    public:
      ShaderBytecode(ShaderCodeTranslator& shdrCode);

      inline DXBCHeader* getHeader() {
        return (DXBCHeader*)getBytecode();
      }

      inline const uint8_t* getBytecode() const {
        return (uint8_t*)&m_bytecode[0];
      }

      inline uint32_t getByteSize() const {
        return m_bytecode.size() * sizeof(uint32_t);
      }

      inline std::vector<uint32_t>& getBytecodeVector() {
        return m_bytecode;
      }
    private:
      std::vector<uint32_t> m_bytecode;
    };

  }

}