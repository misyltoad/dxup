#include "d3d9_state_cache.h"
#define XXH_INLINE_ALL
#include "../extern/xxhash/xxhash.h"

namespace dxup {

  // These hash functions only include states that we change to avoid collisions!
  // Remember to update me if we do change more states!

  template <typename T>
  inline size_t hashDesc(const T& desc) {
#ifdef _WIN32
    return XXH32(&desc, sizeof(T), 0);
#else
    return XXH64(&desc, sizeof(T), 0);
#endif
  }

  size_t D3D11StateDescHash::operator () (const D3D11_SAMPLER_DESC& desc) const {
    return hashDesc(desc);
  }

  size_t D3D11StateDescHash::operator () (const D3D11_RENDER_TARGET_BLEND_DESC1& desc) const {
    return hashDesc(desc);
  }

  size_t D3D11StateDescHash::operator () (const D3D11_BLEND_DESC1& desc) const {
    return hashDesc(desc.RenderTarget[0]);
  }

  size_t D3D11StateDescHash::operator () (const D3D11_RASTERIZER_DESC1& desc) const {
    return hashDesc(desc);
  }

  size_t D3D11StateDescHash::operator () (const D3D11_DEPTH_STENCILOP_DESC& desc) const {
    return hashDesc(desc);
  }

  size_t D3D11StateDescHash::operator () (const D3D11_DEPTH_STENCIL_DESC& desc) const {
    return hashDesc(desc);
  }

  // eq

  bool D3D11StateDescEqual::operator () (const D3D11_BLEND_DESC1& a, const D3D11_BLEND_DESC1& b) const {
    return this->operator () (a.RenderTarget[0], b.RenderTarget[0]);
  }


  bool D3D11StateDescEqual::operator () (const D3D11_DEPTH_STENCILOP_DESC& a, const D3D11_DEPTH_STENCILOP_DESC& b) const {
    return a.StencilFunc == b.StencilFunc
      && a.StencilDepthFailOp == b.StencilDepthFailOp
      && a.StencilPassOp == b.StencilPassOp
      && a.StencilFailOp == b.StencilFailOp;
  }


  bool D3D11StateDescEqual::operator () (const D3D11_DEPTH_STENCIL_DESC& a, const D3D11_DEPTH_STENCIL_DESC& b) const {
    return a.DepthEnable == b.DepthEnable
      && a.DepthWriteMask == b.DepthWriteMask
      && a.DepthFunc == b.DepthFunc
      && a.StencilEnable == b.StencilEnable
      && a.StencilReadMask == b.StencilReadMask
      && a.StencilWriteMask == b.StencilWriteMask
      && this->operator () (a.FrontFace, b.FrontFace)
      && this->operator () (a.BackFace, b.BackFace);
  }


  bool D3D11StateDescEqual::operator () (const D3D11_RASTERIZER_DESC1& a, const D3D11_RASTERIZER_DESC1& b) const {
    return a.FillMode == b.FillMode
      && a.CullMode == b.CullMode
      && a.FrontCounterClockwise == b.FrontCounterClockwise
      && a.DepthBias == b.DepthBias
      && a.SlopeScaledDepthBias == b.SlopeScaledDepthBias
      && a.DepthBiasClamp == b.DepthBiasClamp
      && a.DepthClipEnable == b.DepthClipEnable
      && a.ScissorEnable == b.ScissorEnable
      && a.MultisampleEnable == b.MultisampleEnable
      && a.AntialiasedLineEnable == b.AntialiasedLineEnable
      && a.ForcedSampleCount == b.ForcedSampleCount;
  }


  bool D3D11StateDescEqual::operator () (const D3D11_RENDER_TARGET_BLEND_DESC1& a, const D3D11_RENDER_TARGET_BLEND_DESC1& b) const {
    return a.BlendEnable == b.BlendEnable
      && a.SrcBlend == b.SrcBlend
      && a.DestBlend == b.DestBlend
      && a.BlendOp == b.BlendOp
      && a.SrcBlendAlpha == b.SrcBlendAlpha
      && a.DestBlendAlpha == b.DestBlendAlpha
      && a.BlendOpAlpha == b.BlendOpAlpha
      && a.RenderTargetWriteMask == b.RenderTargetWriteMask;
  }


  bool D3D11StateDescEqual::operator () (const D3D11_SAMPLER_DESC& a, const D3D11_SAMPLER_DESC& b) const {
    return a.Filter == b.Filter
      && a.AddressU == b.AddressU
      && a.AddressV == b.AddressV
      && a.AddressW == b.AddressW
      && a.MipLODBias == b.MipLODBias
      && a.MaxAnisotropy == b.MaxAnisotropy
      && a.BorderColor[0] == b.BorderColor[0]
      && a.BorderColor[1] == b.BorderColor[1]
      && a.BorderColor[2] == b.BorderColor[2]
      && a.BorderColor[3] == b.BorderColor[3]
      && a.MaxLOD == b.MaxLOD;
  }
}