#pragma once

#include "../extern/smhasher/MurmurHash3.h"

namespace dxup {

  template <typename T>
  uint32_t hash(const T& item) {
    uint32_t val;
    MurmurHash3_x86_32(&item, sizeof(T), 0xB0F57EE3, &val);
    return val;
  }

}