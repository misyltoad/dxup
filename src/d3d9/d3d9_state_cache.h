#pragma once
#include <vector>
#include <string>
#include "d3d9_base.h"
#include <unordered_map>

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

    using StateMap = std::unordered_map<Desc, Com<Object>, D3D11StateDescHash, D3D11StateDescEqual>;

    Object* lookupObject(const Desc& desc) {
      StateMap::const_iterator iter = m_map.find(desc);

      if (iter == m_map.end())
        return nullptr;

      return iter->second.ptr();
    }

    void pushState(const Desc& desc, Object* object) {
      m_map.emplace(desc, object);
    }

  private:

    StateMap m_map;
  };

}