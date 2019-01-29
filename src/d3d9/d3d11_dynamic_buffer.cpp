#include "d3d11_dynamic_buffer.h"

namespace dxup {

  D3D11DynamicBuffer::D3D11DynamicBuffer(ID3D11Device* device, uint32_t bindFlags)
    : m_device{ device }
    , m_size{ 0 }
    , m_offset{ 0 }
    , m_bindFlags{ bindFlags } {}

  ID3D11Buffer* D3D11DynamicBuffer::getBuffer() {
    return m_buffer.ptr();
  }

  void D3D11DynamicBuffer::reserve(uint32_t length) {
    if (m_buffer != nullptr && m_size >= m_offset + length)
      return;

    m_buffer = nullptr;

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = m_offset + length;
    desc.BindFlags = m_bindFlags;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    Com<ID3D11Buffer> buffer;
    HRESULT result = m_device->CreateBuffer(&desc, nullptr, &buffer);
    if (FAILED(result)) {
      log::warn("reserve: CreateBuffer failed (length = %d + %d.)", m_offset + length);
      return;
    }

    m_buffer = buffer;
    m_size = m_offset + length;

    m_offset = 0;

    return;
  }

  uint32_t D3D11DynamicBuffer::update(ID3D11DeviceContext* context, const void* src, uint32_t length) {
    void* data;
    this->map(context, &data, length);
    std::memcpy(&data, src, length);
    return this->unmap(context, length);
  }

  void D3D11DynamicBuffer::map(ID3D11DeviceContext* context, void** data, uint32_t length) {
    D3D11_MAPPED_SUBRESOURCE res;
    context->Map(m_buffer.ptr(), 0, m_offset == 0 ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE, 0, &res);
    uint8_t* resourceData = (uint8_t*)res.pData;
    *data = resourceData + m_offset;
  }

  uint32_t D3D11DynamicBuffer::unmap(ID3D11DeviceContext* context, uint32_t length) {
    context->Unmap(m_buffer.ptr(), 0);
    uint32_t offset = m_offset;
    m_offset += length;
    return offset;
  }

  void D3D11DynamicBuffer::endFrame() {
    m_offset = 0;
  }
}