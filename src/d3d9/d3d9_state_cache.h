#pragma once
#include <vector>
#include <string>
#include "d3d9_base.h"

namespace dxup {

  template <typename Desc, typename Object>
  class StateCache {

  public:

    Object* lookupObject(uint32_t key) {
      for (uint32_t i = 0; i < m_keys.size(); i++) {
        if (m_keys[i] == key)
          return m_objects[i].ptr();
      }

      return nullptr;
    }

    void pushState(uint32_t key, Object* object) {
      m_keys.push_back(key);
      m_objects.emplace_back(object);
    }

  private:

    std::vector<uint32_t> m_keys;
    std::vector<Com<Object>> m_objects;
  };

}