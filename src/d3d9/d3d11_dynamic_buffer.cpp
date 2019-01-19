#include "d3d11_dynamic_buffer.h"

namespace dxup {

  D3D11DynamicBuffer::D3D11DynamicBuffer(ID3D11Device* device, uint32_t bindFlags)
    : m_device{ device }
    , m_length{ 0 }
    , m_bindFlags{ bindFlags } {}

  ID3D11Buffer* D3D11DynamicBuffer::getBuffer() {
    return m_buffer.ptr();
  }

  void D3D11DynamicBuffer::reserve(uint32_t length) {
    if (m_buffer != nullptr && m_length >= length)
      return;

    m_buffer = nullptr;

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = length;
    desc.BindFlags = m_bindFlags;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    Com<ID3D11Buffer> buffer;
    HRESULT result = m_device->CreateBuffer(&desc, nullptr, &buffer);
    if (FAILED(result)) {
      log::warn("reserve: CreateBuffer failed (length = %d.)", length);
      return;
    }

    m_buffer = buffer;
    m_length = length;
  }

  void D3D11DynamicBuffer::update(ID3D11DeviceContext* context, const void* src, uint32_t length) {
    D3D11_MAPPED_SUBRESOURCE res;
    context->Map(m_buffer.ptr(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    std::memcpy(res.pData, src, length);
    context->Unmap(m_buffer.ptr(), 0);
  }

}