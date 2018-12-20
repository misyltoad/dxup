#include "d3d9_constant_buffer.h"

namespace dxup {

  struct BufferInfo {
    uint32_t elementSize;
    uint32_t elementCount;
  };

  static std::array<BufferInfo, BufferType::Count> bufferInfo = {
    BufferInfo{sizeof(float) * 4, 256},
    BufferInfo{sizeof(int) * 4, 16},
    BufferInfo{sizeof(int), 16},
  };

  //

  D3D9ConstantBuffer::D3D9ConstantBuffer(ID3D11Device1* device, ID3D11DeviceContext1* context, ShaderType type, BufferType bufferType)
    : m_device{ device }
    , m_context{ context }
    , m_elementSize{ bufferInfo[bufferType].elementSize }
    , m_elementCount{ bufferInfo[bufferType].elementCount } {
    m_elements.reset(new uint8_t[m_elementSize * m_elementCount]);
    std::memset(m_elements.get(), 0, m_elementSize * m_elementCount);

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.ByteWidth = m_elementSize * m_elementCount;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;
    cbDesc.StructureByteStride = 0;
    HRESULT result = m_device->CreateBuffer(&cbDesc, nullptr, &m_buffer);
    if (FAILED(result))
      log::fail("Couldn't create constant buffer.");

    ID3D11Buffer* buffer = m_buffer.ptr();
    if (type == ShaderType::Vertex)
      m_context->VSSetConstantBuffers(bufferType, 1, &buffer);
    else
      m_context->PSSetConstantBuffers(bufferType, 1, &buffer);

    pushData();
  }

  void D3D9ConstantBuffer::prepareDraw() {
    pushData();
  }

  HRESULT D3D9ConstantBuffer::set(uint32_t index, const void* values, uint32_t count) {
    if (values == nullptr)
      return D3DERR_INVALIDCALL;

    if (count == 0)
      return D3D_OK;

    std::memcpy(&m_elements.get()[index * m_elementSize], values, m_elementSize * count);
    return D3D_OK;
  }

  HRESULT D3D9ConstantBuffer::get(uint32_t index, void* values, uint32_t count) {
    if (values == nullptr)
      return D3DERR_INVALIDCALL;

    if (count == 0)
      return D3D_OK;

    std::memcpy(values, (void**)&m_elements.get()[index * m_elementSize], m_elementSize * count);
    return D3D_OK;
  }

  void D3D9ConstantBuffer::pushData() {
    D3D11_MAPPED_SUBRESOURCE res;
    m_context->Map(m_buffer.ptr(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    std::memcpy(res.pData, m_elements.get(), m_elementSize * m_elementCount);
    m_context->Unmap(m_buffer.ptr(), 0);
  }

  //

  D3D9ConstantBuffers::D3D9ConstantBuffers(ID3D11Device1* device, ID3D11DeviceContext1* context) {
    for (uint32_t i = 0; i < ShaderType::Count; i++) {
      for (uint32_t j = 0; j < BufferType::Count; j++)
        m_buffers[(j * ShaderType::Count) + i].reset(new D3D9ConstantBuffer{ device, context, (ShaderType)i, (BufferType)j });
    }
  }

  void D3D9ConstantBuffers::prepareDraw() {
    for (uint32_t i = 0; i < ShaderType::Count; i++) {
      for (uint32_t j = 0; j < BufferType::Count; j++)
        m_buffers[(j * ShaderType::Count) + i]->prepareDraw();
    }
  }

  HRESULT D3D9ConstantBuffers::set(ShaderType shdrType, BufferType bufferType, uint32_t index, const void* values, uint32_t count) {
    return getBuffer(shdrType, bufferType).set(index, values, count);
  }

  HRESULT D3D9ConstantBuffers::get(ShaderType shdrType, BufferType bufferType, uint32_t index, void* values, uint32_t count) {
    return getBuffer(shdrType, bufferType).get(index, values, count);
  }

  D3D9ConstantBuffer& D3D9ConstantBuffers::getBuffer(ShaderType shdrType, BufferType bufferType) {
    return *m_buffers[(bufferType * ShaderType::Count) + shdrType];
  }
}