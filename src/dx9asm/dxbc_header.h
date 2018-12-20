#pragma once
#include "../util/fourcc.h"

namespace dxup {

  namespace dx9asm {

    namespace chunks {
      enum {
        RDEF = 0,
        ISGN = 1,
        OSGN,
        SHEX,
        STAT,
        Count
      };
    }

    struct DXBCHeader {
      uint32_t dxbc = fourcc("DXBC");
      uint32_t checksum[4] = { 0 };
      uint32_t unknown = 1;
      uint32_t size = 0; // Set later.
      uint32_t chunkCount = chunks::Count;
      uint32_t chunkOffsets[chunks::Count] = { 0 }; // Set later.
    };

  }

}