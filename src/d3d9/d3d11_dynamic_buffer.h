#pragma once

#include "d3d9_base.h"

namespace dxup {

  class D3D11DynamicBuffer {

  public:

    D3D11DynamicBuffer(ID3D11Device* device, bool index);

    void reserve(uint32_t length);
    void update(ID3D11DeviceContext* context, const void* src, uint32_t length);

    ID3D11Buffer* getBuffer();

  private:

    ID3D11Device* m_device;
    Com<ID3D11Buffer> m_buffer;
    uint32_t m_length;
    bool m_index;

  };

}