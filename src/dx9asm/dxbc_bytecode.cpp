#include "dxbc_bytecode.h"
#include "dx9asm_translator.h"
#include "dxbc_chunks.h"
#include "dxbc_header.h"
#include "../util/misc_helpers.h"
#include "../extern/gpuopen/DXBCChecksum.h"

namespace dxapex {

  namespace dx9asm {

    ShaderBytecode::ShaderBytecode(ShaderCodeTranslator& shdrCode) {
      // Should be enough to avoid any extra allocations.
      m_bytecode.reserve(4096 + shdrCode.getCode().size());

      pushObject(m_bytecode, DXBCHeader());

      getHeader()->chunkOffsets[chunks::RDEF] = getByteSize();
      writeRDEF(*this, shdrCode);

      getHeader()->chunkOffsets[chunks::ISGN] = getByteSize();
      writeISGN(*this, shdrCode);

      getHeader()->chunkOffsets[chunks::OSGN] = getByteSize();
      writeOSGN(*this, shdrCode);

      getHeader()->chunkOffsets[chunks::SHEX] = getByteSize();
      writeSHEX(*this, shdrCode);

      getHeader()->chunkOffsets[chunks::STAT] = getByteSize();
      writeSTAT(*this, shdrCode);

      getHeader()->size = getByteSize();
      
      BOOL success = CalculateDXBCChecksum(getBytecode(), getByteSize(), (DWORD*)getHeader()->checksum);
      if (!success)
        log::fail("Couldn't calculate DXBC checksum!");
    }

  }

}