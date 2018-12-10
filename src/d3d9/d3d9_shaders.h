#pragma once

#include "d3d9_base.h"
#include "d3d9_device_unknown.h"
#include <vector>
#include "../dx9asm/dx9asm_translator.h"

namespace dxapex {

  struct InputLink {
    Com<ID3D11InputLayout> inputLayout;
    Com<IDirect3DVertexDeclaration9> vertexDcl;
  };

  template <dx9asm::ShaderType Type, typename D3D11Shader, typename Base>
  class Direct3DShader9 final : public D3D9DeviceUnknown<Base> {

  public:

    Direct3DShader9(Direct3DDevice9Ex* device, const DWORD* code, D3D11Shader* shader, dx9asm::ShaderBytecode* translation)
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

    const dx9asm::ShaderBytecode* GetTranslation() const {
      return m_translation;
    }

    void GetD3D11Shader(D3D11Shader** shader) {
      if (shader != nullptr)
        *shader = ref(m_shader);
    }

    void LinkInput(ID3D11InputLayout* inputLayout, IDirect3DVertexDeclaration9* vertDcl) {
      m_inputLinks.push_back(InputLink{ inputLayout, vertDcl });
    }

    void GetLinkedInput(IDirect3DVertexDeclaration9* vertDcl, ID3D11InputLayout** inputLayout) {
      if (inputLayout == nullptr || vertDcl == nullptr)
        return;

      InitReturnPtr(inputLayout);

      for (InputLink& link : m_inputLinks) {
        if (link.vertexDcl == vertDcl)
          *inputLayout = ref(link.inputLayout);
      }

      return;
    }

  private:

    std::vector<InputLink> m_inputLinks;
    std::vector<uint32_t> m_dx9asm;
    Com<D3D11Shader> m_shader;
    const dx9asm::ShaderBytecode* m_translation;
  };

  using Direct3DVertexShader9 = Direct3DShader9<dx9asm::ShaderType::Vertex, ID3D11VertexShader, IDirect3DVertexShader9>;

}