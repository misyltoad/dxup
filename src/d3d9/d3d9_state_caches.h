#pragma once

#include "d3d9_base.h"
#include "d3d9_state_cache.h"

namespace dxup {

  struct D3D9StateCaches {
    StateCache<D3D11_RASTERIZER_DESC1, ID3D11RasterizerState1> rasterizer;
    StateCache<D3D11_BLEND_DESC1, ID3D11BlendState1> blendState;
    StateCache<D3D11_DEPTH_STENCIL_DESC, ID3D11DepthStencilState> depthStencil;
    StateCache<D3D11_SAMPLER_DESC, ID3D11SamplerState> sampler;
  };

}