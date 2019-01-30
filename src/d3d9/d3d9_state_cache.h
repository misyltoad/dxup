#pragma once
#include <vector>
#include <string>
#include "d3d9_base.h"

#include "../util/hash.h"

namespace dxup {

  struct D3D11StateDescHash {
    size_t operator () (const D3D11_SAMPLER_DESC& desc) const;
    size_t operator () (const D3D11_RENDER_TARGET_BLEND_DESC1& desc) const;
    size_t operator () (const D3D11_BLEND_DESC1& desc) const;
    size_t operator () (const D3D11_RASTERIZER_DESC1& desc) const;
    size_t operator () (const D3D11_DEPTH_STENCILOP_DESC& desc) const;
    size_t operator () (const D3D11_DEPTH_STENCIL_DESC& desc) const;
  };

  struct D3D11StateDescEqual {
    bool operator () (const D3D11_SAMPLER_DESC& a, const D3D11_SAMPLER_DESC& b) const;
    bool operator () (const D3D11_RENDER_TARGET_BLEND_DESC1& a, const D3D11_RENDER_TARGET_BLEND_DESC1& b) const;
    bool operator () (const D3D11_BLEND_DESC1& a, const D3D11_BLEND_DESC1& b) const;
    bool operator () (const D3D11_RASTERIZER_DESC1& a, const D3D11_RASTERIZER_DESC1& b) const;
    bool operator () (const D3D11_DEPTH_STENCILOP_DESC& a, const D3D11_DEPTH_STENCILOP_DESC& b) const;
    bool operator () (const D3D11_DEPTH_STENCIL_DESC& a, const D3D11_DEPTH_STENCIL_DESC& b) const;
  };

  template <typename Desc, typename Object>
  class StateCache {

  public:

    using StatePair = std::pair<size_t, Com<Object>>;
    
    size_t hash(const Desc& desc) {
      static D3D11StateDescHash hasher;
      return hasher.operator()(desc);
    }

    Object* lookupObject(size_t hash) {
      for (const auto& obj : m_objects) {
        if (obj.first == hash)
          return obj.second.ptr();
      }
      return nullptr;
    }

    void pushState(size_t hash, const Desc& desc, Object* object) {
      StatePair pair = StatePair{ hash, object };
      m_objects.push_back(pair);
    }

  private:

    std::vector<StatePair> m_objects;

  };

}