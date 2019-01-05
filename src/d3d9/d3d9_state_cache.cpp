#include "d3d9_state_cache.h"

namespace dxup {

  // These hash functions only include states that we change to avoid collisions!
  // Remember to update me if we do change more states!

  size_t D3D11StateDescHash::operator () (const D3D11_SAMPLER_DESC& desc) const {
    HashState hash;
    std::hash<float> fhash;

    hash.add(desc.AddressU);
    hash.add(desc.AddressV);
    hash.add(desc.AddressW);
    for (uint32_t i = 0; i < 4; i++)
      hash.add(fhash(desc.BorderColor[i]));
    hash.add(desc.Filter);
    hash.add(desc.MaxAnisotropy);
    hash.add(fhash(desc.MipLODBias));
    hash.add(fhash(desc.MaxLOD));

    return hash;
  }

  size_t D3D11StateDescHash::operator () (const D3D11_RENDER_TARGET_BLEND_DESC1& desc) const {
    HashState hash;
    hash.add(desc.BlendEnable);
    hash.add(desc.SrcBlend);
    hash.add(desc.DestBlend);
    hash.add(desc.BlendOp);
    hash.add(desc.SrcBlendAlpha);
    hash.add(desc.DestBlendAlpha);
    hash.add(desc.BlendOpAlpha);
    hash.add(desc.RenderTargetWriteMask);
    return hash;
  }

  size_t D3D11StateDescHash::operator () (const D3D11_BLEND_DESC1& desc) const {
    HashState hash;
    // For now we only use one.
    hash.add(this->operator()(desc.RenderTarget[0]));
    return hash;
  }

  size_t D3D11StateDescHash::operator () (const D3D11_RASTERIZER_DESC1& desc) const {
    std::hash<float> fhash;

    HashState hash;
    hash.add(desc.FillMode);
    hash.add(desc.CullMode);
    hash.add(desc.DepthBias);
    hash.add(fhash(desc.SlopeScaledDepthBias));
    hash.add(fhash(desc.DepthBiasClamp));
    hash.add(desc.ScissorEnable);
    hash.add(desc.MultisampleEnable); // Sample stuff is included even though we don't use it because we probably will soon.
    hash.add(desc.AntialiasedLineEnable);
    hash.add(desc.ForcedSampleCount);
    return hash;
  }

  size_t D3D11StateDescHash::operator () (const D3D11_DEPTH_STENCILOP_DESC& desc) const {
    HashState hash;
    hash.add(desc.StencilFunc);
    hash.add(desc.StencilDepthFailOp);
    hash.add(desc.StencilPassOp);
    hash.add(desc.StencilFailOp);
    return hash;
  }

  size_t D3D11StateDescHash::operator () (const D3D11_DEPTH_STENCIL_DESC& desc) const {
    HashState hash;
    hash.add(desc.DepthEnable);
    hash.add(desc.DepthWriteMask);
    hash.add(desc.DepthFunc);
    hash.add(desc.StencilEnable);
    hash.add(desc.StencilReadMask);
    hash.add(desc.StencilWriteMask);
    hash.add(this->operator () (desc.FrontFace));
    hash.add(this->operator () (desc.BackFace));
    return hash;
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