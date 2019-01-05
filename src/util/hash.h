#pragma once

// HashState taken from DXVK.
// https://github.com/doitsujin/dxvk/blob/master/src/dxvk/dxvk_hash.h

#include <cstddef>

namespace dxup {
  
  class HashState {
    
  public:
    
    inline void add(size_t hash) {
      m_value ^= hash + 0x9e3779b9
               + (m_value << 6)
               + (m_value >> 2);
    }
    
    operator size_t () const {
      return m_value;
    }
    
  private:
    
    size_t m_value = 0;
    
  };

}