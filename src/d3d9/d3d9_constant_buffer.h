#pragma once

#include "../dx9asm/dx9asm_meta.h"
#include "d3d9_base.h"
#include <array>
#include <memory>

namespace dxup {

  using ShaderType = dx9asm::ShaderType;

  enum BufferType {
    Float = 0,
    Int = 1,
    Bool,
    Count,
  };

  class D3D9ConstantBuffer {

  public:

    static constexpr size_t singleFloatConstantSize = sizeof(float) * 4;
    static constexpr size_t floatConstantCount = 256;
    static constexpr size_t floatElementsSize = singleFloatConstantSize * floatConstantCount;

    static constexpr size_t singleIntConstantSize = sizeof(int) * 4;
    static constexpr size_t intConstantCount = 16;
    static constexpr size_t intElementsSize = singleIntConstantSize * intConstantCount;

    static constexpr size_t singleBoolConstantSize = sizeof(int);
    static constexpr size_t boolConstantCount = 16;
    static constexpr size_t boolElementsSize = singleBoolConstantSize * boolConstantCount;

    D3D9ConstantBuffer(ID3D11Device1* device, ID3D11DeviceContext1* context, bool pixelShader);
    ~D3D9ConstantBuffer();

    void prepareDraw();
    void bind();

    HRESULT set(BufferType type, uint32_t index, const void* values, uint32_t count);

    HRESULT get(BufferType type, uint32_t index, void* values, uint32_t count);

  private:

    void pushData();

    // I exist as long as my parent D3D9 device exists. No need for COM.
    ID3D11Device1* m_device;
    ID3D11DeviceContext1* m_context;

    float* m_floatElements;
    int* m_intElements;
    bool* m_boolElements;

    bool m_pixelShader;

    Com<ID3D11Buffer> m_buffer;

    bool m_dirty;
  };

}