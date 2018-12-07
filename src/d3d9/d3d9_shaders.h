#pragma once

#include "d3d9_base.h"
#include "d3d9_device_unknown.h"
#include <vector>
#include "../dx9asm/dx9asm_translator.h"

namespace dxapex {

  template <dx9asm::ShaderType Type, typename... Base>
  class Direct3DShader9 final : public D3D9DeviceUnknown<Base...> {

  public:

    Direct3DShader9(uint32_t* code)
      : m_dxbc{ dxbc }  {

      m_d3d9code.resize(dx9asm::byteCodeLength(code));
      std::memcpy(&m_d3d9code[0], code, m_d3d9code.size());
    }

    HRESULT STDMETHODCALLTYPE GetFunction(void* pShader, UINT* pSizeOfData) override {
      if (pSizeOfData == nullptr)
        return D3DERR_INVALIDCALL;

      if (pShader == nullptr) {
        *pSizeOfData = m_d3d9code.size();
        return D3D_OK;
      }
    }

  private:

    std::vector<uint32_t> m_d3d9code;

    //Com<dx9asm::ITranslatedShaderDXBC> m_dxbc;

  };

  using Direct3DVertexShader9 = Direct3DShader9<dx9asm::ShaderType::Vertex, IDirect3DVertexShader9>;

}