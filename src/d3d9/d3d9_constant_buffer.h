#pragma once

#include "../dx9asm/dx9asm_meta.h"
#include "d3d9_base.h"
#include <array>
#include <memory>

namespace dxup {

  struct D3D9ShaderConstants;

  class D3D9ConstantBuffer {

  public:

    D3D9ConstantBuffer(ID3D11Device1* device, ID3D11DeviceContext1* context, bool pixelShader);

    void update(const D3D9ShaderConstants& constants);
    void bind();

  private:

    // I exist as long as my parent D3D9 device exists. No need for COM.
    ID3D11Device1* m_device;
    ID3D11DeviceContext1* m_context;

    bool m_pixelShader;

    Com<ID3D11Buffer> m_buffer;
  };

}