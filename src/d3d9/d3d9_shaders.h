#pragma once

#include "d3d9_base.h"
#include "d3d9_device_unknown.h"
#include <vector>
#include "../dx9asm/dx9asm_translator.h"

namespace dxapex {

  template <dx9asm::ShaderType Type, typename Base>
  class Direct3DShader9 final : public D3D9DeviceUnknown<Base> {

  public:

    Direct3DShader9(Direct3DDevice9Ex* device, const DWORD* code, ID3D11VertexShader* shader, dx9asm::ShaderBytecode* translation)
      : m_shader { shader }
      , m_translation{ translation }
      , D3D9DeviceUnknown<Base>(device) {

      m_dx9asm.resize(dx9asm::byteCodeLength((const uint32_t*)code));

      std::memcpy(&m_dx9asm[0], code, m_dx9asm.size());
    }

    HRESULT STDMETHODCALLTYPE GetFunction(void* pShader, UINT* pSizeOfData) override {
      if (pSizeOfData == nullptr)
        return D3DERR_INVALIDCALL;

      if (pShader == nullptr) {
        *pSizeOfData = m_dx9asm.size();
        return D3D_OK;
      }

      UINT size = *pSizeOfData;
      if (size > m_dx9asm.size())
        size = m_dx9asm.size();

      std::memcpy(pShader, &m_dx9asm[0], (size_t)size);
      return D3D_OK;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
      InitReturnPtr(ppv);

      if (ppv == nullptr)
        return E_POINTER;

      if (riid == __uuidof(Base) || riid == __uuidof(IUnknown))
        *ppv = ref(this);

      return E_NOINTERFACE;
    }

  private:

    std::vector<uint32_t> m_dx9asm;
    Com<ID3D11VertexShader> m_shader;
    dx9asm::ShaderBytecode* m_translation;
  };

  using Direct3DVertexShader9 = Direct3DShader9<dx9asm::ShaderType::Vertex, IDirect3DVertexShader9>;

}