#include "d3d9_constant_buffer.h"
#include "d3d9_state.h"

namespace dxup {

  D3D9ConstantBuffer::D3D9ConstantBuffer(ID3D11Device1* device, ID3D11DeviceContext1* context, bool pixelShader)
    : m_device{ device }
    , m_context{ context }
    , m_pixelShader{ pixelShader } {
    D3D11_BUFFER_DESC cbDesc;
    cbDesc.ByteWidth = sizeof(D3D9ShaderConstants::floatConstants) + sizeof(D3D9ShaderConstants::intConstants); //+ sizeof(uint32_t); // bool elements are encoded as a bitfield but not impl yet.
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

  void D3D9ConstantBuffer::bind() {
    ID3D11Buffer* buffer = m_buffer.ptr();
    if (!m_pixelShader)
      m_context->VSSetConstantBuffers(0, 1, &buffer);
    else
      m_context->PSSetConstantBuffers(0, 1, &buffer);
  }

  void D3D9ConstantBuffer::update(const D3D9ShaderConstants& constants) {
    D3D11_MAPPED_SUBRESOURCE res;
    m_context->Map(m_buffer.ptr(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    // This can probably be consolidated into a single one.
    uint8_t* data = (uint8_t*)res.pData;
    std::memcpy(data, constants.floatConstants.data(), sizeof(constants.floatConstants));
    std::memcpy(data + sizeof(constants.floatConstants), constants.intConstants.data(), sizeof(constants.intConstants));

    m_context->Unmap(m_buffer.ptr(), 0);
  }

}