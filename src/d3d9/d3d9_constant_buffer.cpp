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

  D3D9ConstantBuffer::D3D9ConstantBuffer(ID3D11Device1* device, ID3D11DeviceContext1* context, ShaderType shaderType, BufferType bufferType)
    : m_device{ device }
    , m_context{ context }
    , m_elementSize{ bufferInfo[bufferType].elementSize }
    , m_elementCount{ bufferInfo[bufferType].elementCount }
    , m_shaderType{ shaderType }
    , m_bufferType{ bufferType } {
    std::vector<uint8_t> blankData;
    blankData.resize(m_elementSize * m_elementCount);
    std::memset(blankData.data(), 0, m_elementSize * m_elementCount);

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.ByteWidth = m_elementSize * m_elementCount;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbDesc.MiscFlags = 0;
    cbDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = (void*)blankData.data();
    data.SysMemPitch = m_elementSize * m_elementCount;
    data.SysMemSlicePitch = 0;

    HRESULT result = m_device->CreateBuffer(&cbDesc, &data, &m_buffer);
    if (FAILED(result))
      log::fail("Couldn't create constant buffer.");

    makeStagingDesc(cbDesc, 0, D3DFMT_UNKNOWN);
    result = m_device->CreateBuffer(&cbDesc, &data, &m_stagingBuffer);
    if (FAILED(result))
      log::fail("Couldn't create staging constant buffer.");

    m_dirtyBox.top = 0;
    m_dirtyBox.front = 0;
    m_dirtyBox.bottom = 1;
    m_dirtyBox.back = 1;

    bind();
    finishDraw();
  }

  void D3D9ConstantBuffer::bind() {
    ID3D11Buffer* buffer = m_buffer.ptr();
    if (m_shaderType == ShaderType::Vertex)
      m_context->VSSetConstantBuffers(m_bufferType, 1, &buffer);
    else
      m_context->PSSetConstantBuffers(m_bufferType, 1, &buffer);
  }

  void D3D9ConstantBuffer::prepareDraw() {
    m_context->Unmap(m_stagingBuffer.ptr(), 0);
    pushData();
  }

  void D3D9ConstantBuffer::finishDraw() {
    m_dirtyBox.left = UINT32_MAX;
    m_dirtyBox.right = 0;
    m_context->Map(m_stagingBuffer.ptr(), 0, D3D11_MAP_WRITE, 0, &m_stagingMapping);
  }

  HRESULT D3D9ConstantBuffer::set(uint32_t index, const void* values, uint32_t count) {
    if (values == nullptr)
      return D3DERR_INVALIDCALL;

    if (count == 0)
      return D3D_OK;

    m_dirtyBox.left = std::min(index * m_elementSize, m_dirtyBox.left);
    m_dirtyBox.right = std::max((index + count) * m_elementSize, m_dirtyBox.right);

    uint8_t* data = (uint8_t*)m_stagingMapping.pData;
    std::memcpy((void*)&data[index * m_elementSize], values, m_elementSize * count);
    return D3D_OK;
  }

  HRESULT D3D9ConstantBuffer::get(uint32_t index, void* values, uint32_t count) {
    if (values == nullptr)
      return D3DERR_INVALIDCALL;

    if (count == 0)
      return D3D_OK;

    uint8_t* data = (uint8_t*)m_stagingMapping.pData;
    std::memcpy(values, (void*)&data[index * m_elementSize], m_elementSize * count);
    return D3D_OK;
  }

  void D3D9ConstantBuffer::pushData() {
    if (m_dirtyBox.left == UINT32_MAX)
      return;

    m_context->CopySubresourceRegion(m_buffer.ptr(), 0, m_dirtyBox.left, 0, 0, m_stagingBuffer.ptr(), 0, &m_dirtyBox);
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

  void D3D9ConstantBuffers::finishDraw() {
    for (uint32_t i = 0; i < ShaderType::Count; i++) {
      for (uint32_t j = 0; j < BufferType::Count; j++)
        m_buffers[(j * ShaderType::Count) + i]->finishDraw();
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