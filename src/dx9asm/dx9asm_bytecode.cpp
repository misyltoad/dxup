#include "dx9asm_translator.h"
#include "dx9asm_operation_helpers.h"
#include "dxbc_chunks.h"
#include "dxbc_header.h"
#include "../util/misc_helpers.h"

namespace dxapex {

  namespace dx9asm {

    class ShaderBytecode {
    public:
      ShaderBytecode(ShaderCodeTranslator& shdrCode) {
        // Should be enough to avoid any extra allocations.
        m_bytecode.reserve(4096 + shdrCode.getCode().size());

        pushObject(m_bytecode, DXBCHeader());

        getHeader()->chunkOffsets[chunks::RDEF] = getByteSize();
        RDEFChunk(shdrCode).push(m_bytecode);

        getHeader()->chunkOffsets[chunks::ISGN] = getByteSize();
        IOSGNChunk<false>(shdrCode).push(m_bytecode);

        getHeader()->chunkOffsets[chunks::OSGN] = getByteSize();
        IOSGNChunk<true>(shdrCode).push(m_bytecode);

        getHeader()->chunkOffsets[chunks::SHDR] = getByteSize();
        SHDRChunk(shdrCode).push(m_bytecode);

        getHeader()->chunkOffsets[chunks::STAT] = getByteSize();
        STATChunk(shdrCode).push(m_bytecode);

        getHeader()->size = getByteSize();
        // Do Checksum Here
      }

      DXBCHeader* getHeader() {
        return (DXBCHeader*)&m_bytecode[0];
      }

      uint32_t getByteSize() {
        return m_bytecode.size() * sizeof(uint32_t);
      }
    private:
      std::vector<uint32_t> m_bytecode;
    };

  }

}