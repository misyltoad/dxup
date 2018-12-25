#pragma once
#include <vector>
#include <string>
#include "d3d9_base.h"

namespace dxup {

  template <typename Desc, typename Object>
  class StateCache {

    struct Instance {
      Desc desc;
      Com<Object> object;
    };

    bool descEqual(const Desc& a, const Desc& b) {
      return std::memcmp(&a, &b, sizeof(Desc)) == 0;
    }

  public:

    Object* lookupObject(const Desc& desc) {
      for (const Instance& instance : m_instances) {
        if (descEqual(instance.desc, desc))
          return instance.object.ptr();
      }

      return nullptr;
    }

    void pushState(const Desc& desc, Object* object) {
      m_instances.push_back(Instance{ desc, object });
    }

  private:

    std::vector<Instance> m_instances;
  };

}