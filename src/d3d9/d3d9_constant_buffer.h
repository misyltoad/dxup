#pragma once

#include "../dx9asm/dx9asm_meta.h"
#include "d3d9_base.h"
#include <array>
#include <memory>
#include <cstring>
#include "../util/vectypes.h"
#include "d3d11_dynamic_buffer.h"

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
      , m_context{ context }
      , m_buffer{ device, D3D11_BIND_CONSTANT_BUFFER }
      , m_offset{ 0 } {
    }

    constexpr uint32_t getConstantSize() {
      return 4 * sizeof(float);
    }

    constexpr uint32_t getLength() {
      uint32_t length = sizeof(D3D9ShaderConstants::floatConstants) + sizeof(D3D9ShaderConstants::intConstants) + (4 * sizeof(D3D9ShaderConstants::boolConstants));
      return alignTo(length, 16 * getConstantSize());
    }

    constexpr uint32_t getConstantCount() {
      return getLength() / (getConstantSize());
    }

    void update(const D3D9ShaderConstants& constants) {
      const uint32_t length = getLength();
      m_buffer.reserve(length); // TODO make bool constants a bitfield.

      uint8_t* data;
      m_buffer.map(m_context, (void**)(&data), length);

      // This can probably be consolidated into a single one.
      std::memcpy(data, constants.floatConstants.data(), sizeof(constants.floatConstants));
      std::memcpy(data + sizeof(constants.floatConstants), constants.intConstants.data(), sizeof(constants.intConstants));

      int* boolData = (int*)(data + sizeof(constants.floatConstants) + sizeof(constants.intConstants));
      for (uint32_t i = 0; i < constants.boolConstants.size(); i++) {
        for (uint32_t j = 0; j < 4; j++)
          boolData[i * 4 + j] = constants.boolConstants[i];
      }

      m_offset = m_buffer.unmap(m_context, length);
      bind();
    }

    void bind() {
      const uint32_t constantOffset = m_offset / getConstantSize();
      const uint32_t constantCount = getConstantCount();

      ID3D11Buffer* buffer = m_buffer.getBuffer();

      if constexpr (Pixel)
        m_context->PSSetConstantBuffers1(0, 1, &buffer, &constantOffset, &constantCount);
      else
        m_context->VSSetConstantBuffers1(0, 1, &buffer, &constantOffset, &constantCount);
    }

    void endFrame() {
      m_buffer.endFrame();
    }

  private:

    // I exist as long as my parent D3D9 device exists. No need for COM.
    ID3D11Device1* m_device;
    ID3D11DeviceContext1* m_context;

    D3D11DynamicBuffer m_buffer;
    uint32_t m_offset;
  };

}