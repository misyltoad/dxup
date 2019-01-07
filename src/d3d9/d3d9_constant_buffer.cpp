#include "d3d9_constant_buffer.h"

namespace dxup {

  inline bool copyConstants(bool dirty, void* dst, void const* src, size_t size) {
    //if (!dirty) {
    //  int diff = std::memcmp(dst, src, size);
    //  dirty = diff != 0;
    //}

    //if (dirty)
      std::memcpy(dst, src, size);

    //return dirty;
    return true;
  }

  D3D9ConstantBuffer::D3D9ConstantBuffer(ID3D11Device1* device, ID3D11DeviceContext1* context, bool pixelShader)
    : m_device{ device }
    , m_context{ context }
    , m_pixelShader{ pixelShader }
    , m_dirty{ true } {

    m_floatElements = (uint8_t*) _aligned_malloc(floatElementsSize, 16);
    std::memset(m_floatElements, 0, floatElementsSize);

    m_intElements = (uint8_t*) _aligned_malloc(intElementsSize, 16);
    std::memset(m_intElements, 0, intElementsSize);

    m_boolElements = (uint8_t*) _aligned_malloc(boolElementsSize, 16);
    std::memset(m_boolElements, 0, boolElementsSize);

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.ByteWidth = floatElementsSize + intElementsSize; //+ sizeof(uint32_t); // bool elements are encoded as a bitfield but not impl yet.
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;
    cbDesc.StructureByteStride = 0;

    HRESULT result = m_device->CreateBuffer(&cbDesc, nullptr, &m_buffer);
    if (FAILED(result))
      log::fail("Couldn't create constant buffer.");

    bind();
    prepareDraw();
  }

  D3D9ConstantBuffer::~D3D9ConstantBuffer() {
    _aligned_free(m_floatElements);
    _aligned_free(m_intElements);
    _aligned_free(m_boolElements);
  }

  void D3D9ConstantBuffer::bind() {
    ID3D11Buffer* buffer = m_buffer.ptr();
    if (!m_pixelShader)
      m_context->VSSetConstantBuffers(0, 1, &buffer);
    else
      m_context->PSSetConstantBuffers(0, 1, &buffer);
  }

  void D3D9ConstantBuffer::prepareDraw() {
    if (m_dirty)//|| dirtyShader)
      pushData();
  }

  HRESULT D3D9ConstantBuffer::set(BufferType type, uint32_t index, const void* values, uint32_t count) {
    if (values == nullptr)
      return D3DERR_INVALIDCALL;

    if (count == 0)
      return D3D_OK;

    if (type == BufferType::Float)
      m_dirty = copyConstants(m_dirty, (void*)&m_floatElements[index * singleFloatConstantSize], values, singleFloatConstantSize * count);
    else if (type == BufferType::Int)
      m_dirty = copyConstants(m_dirty, (void*)&m_intElements[index * singleIntConstantSize], values, singleIntConstantSize * count);
    else if (type == BufferType::Bool)
      m_dirty = copyConstants(m_dirty, (void*)&m_boolElements[index * singleBoolConstantSize], values, singleBoolConstantSize * count);

    return D3D_OK;
  }

  HRESULT D3D9ConstantBuffer::get(BufferType type, uint32_t index, void* values, uint32_t count) {
    if (values == nullptr)
      return D3DERR_INVALIDCALL;

    if (count == 0)
      return D3D_OK;

    if (type == BufferType::Float)
      std::memcpy(values, (void*)&m_floatElements[index * singleFloatConstantSize], singleFloatConstantSize * count);
    else if (type == BufferType::Int)
      std::memcpy(values, (void*)&m_intElements[index * singleIntConstantSize], singleIntConstantSize * count);
    else if (type == BufferType::Bool)
      std::memcpy(values, (void*)&m_boolElements[index * singleBoolConstantSize], singleBoolConstantSize * count);

    return D3D_OK;
  }

  void D3D9ConstantBuffer::pushData() {
    D3D11_MAPPED_SUBRESOURCE res;
    m_context->Map(m_buffer.ptr(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

    // This can probably be consolidated into a single one.
    uint8_t* data = (uint8_t*)res.pData;
    std::memcpy(data, m_floatElements, floatElementsSize);
    std::memcpy(data + floatElementsSize, m_intElements, intElementsSize);

    m_context->Unmap(m_buffer.ptr(), 0);

    m_dirty = false;
  }
}