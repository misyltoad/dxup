#include "dxbc_bytecode.h"
#include "dx9asm_translator.h"
#include "dx9asm_operation_helpers.h"
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
      RDEFChunk(*this).push();

      getHeader()->chunkOffsets[chunks::ISGN] = getByteSize();
      IOSGNChunk<chunks::ISGN>(*this, shdrCode).push();

      getHeader()->chunkOffsets[chunks::OSGN] = getByteSize();
      IOSGNChunk<chunks::OSGN>(*this, shdrCode).push();

      getHeader()->chunkOffsets[chunks::SHEX] = getByteSize();
      SHEXChunk(*this, shdrCode).push();

      getHeader()->chunkOffsets[chunks::STAT] = getByteSize();
      STATChunk(*this).push();

      getHeader()->size = getByteSize();
      
      BOOL success = CalculateDXBCChecksum(getBytecode(), getByteSize(), (DWORD*)getHeader()->checksum);
      if (!success)
        log::fail("Couldn't calculate DXBC checksum!");
    }

  }

}