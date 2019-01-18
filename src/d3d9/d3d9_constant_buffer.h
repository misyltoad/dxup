#pragma once

#include "../dx9asm/dx9asm_meta.h"
#include "d3d9_base.h"
#include <array>
#include <memory>
#include <cstring>
#include "../util/vectypes.h"

namespace dxup {

  struct D3D9ShaderConstants {
    D3D9ShaderConstants() {
      std::memset(floatConstants.data(), 0, floatConstants.size() * sizeof(floatConstants[0]));
      std::memset(intConstants.data(), 0, intConstants.size() * sizeof(intConstants[0]));
      std::memset(boolConstants.data(), 0, boolConstants.size() * sizeof(boolConstants[0]));
    }

    std::array<Vector<float, 4>, 256> floatConstants;
    std::array<Vector<int, 4>, 16> intConstants;
    std::array<int, 16> boolConstants;
  };

  template <bool Pixel>
  class D3D9ConstantBuffer {

  public:

    D3D9ConstantBuffer(ID3D11Device1* device, ID3D11DeviceContext1* context)
      : m_device{ device }
      , m_context{ context } {
      D3D11_BUFFER_DESC cbDesc;
      cbDesc.ByteWidth = sizeof(D3D9ShaderConstants::floatConstants) + sizeof(D3D9ShaderConstants::intConstants) + sizeof(D3D9ShaderConstants::boolConstants); // TODO make bool constants a bitfield.
      cbDesc.Usage = D3D11_USAGE_DYNAMIC;
      cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      cbDesc.MiscFlags = 0;
      cbDesc.StructureByteStride = 0;

      HRESULT result = m_device->CreateBuffer(&cbDesc, nullptr, &m_buffer);
      if (FAILED(result))
        log::fail("Couldn't create constant buffer.");

      bind();
    }

    void update(const D3D9ShaderConstants& constants) {
      D3D11_MAPPED_SUBRESOURCE res;
      m_context->Map(m_buffer.ptr(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

      // This can probably be consolidated into a single one.
      uint8_t* data = (uint8_t*)res.pData;
      std::memcpy(data, constants.floatConstants.data(), sizeof(constants.floatConstants));
      std::memcpy(data + sizeof(constants.floatConstants), constants.intConstants.data(), sizeof(constants.intConstants));
      std::memcpy(data + sizeof(constants.floatConstants) + sizeof(constants.intConstants), constants.boolConstants.data(), sizeof(constants.boolConstants));

      m_context->Unmap(m_buffer.ptr(), 0);
    }

    void bind() {
      ID3D11Buffer* buffer = m_buffer.ptr();
      if constexpr (Pixel)
        m_context->PSSetConstantBuffers(0, 1, &buffer);
      else
        m_context->VSSetConstantBuffers(0, 1, &buffer);
    }

  private:

    // I exist as long as my parent D3D9 device exists. No need for COM.
    ID3D11Device1* m_device;
    ID3D11DeviceContext1* m_context;

    Com<ID3D11Buffer> m_buffer;
  };

}