#pragma once

#include "d3d9_base.h"
#include "d3d9_device_unknown.h"
#include <vector>
#include "../dx9asm/dx9asm_translator.h"

namespace dxup {

  struct InputLink {
    Com<ID3D11InputLayout> inputLayout;
    Com<IDirect3DVertexDeclaration9> vertexDcl;
  };

  template <typename D3D11Shader, typename Base>
  class Direct3DShader9 final : public D3D9DeviceUnknown<Base> {

  public:

    Direct3DShader9(uint32_t shaderNum, Direct3DDevice9Ex* device, const DWORD* code, D3D11Shader* shader, dx9asm::ShaderBytecode* translation)
      : m_shader { shader }
      , m_shaderNum{ shaderNum }
      , m_translation{ translation }
      , D3D9DeviceUnknown<Base>{device} {

      m_dx9asm.resize(dx9asm::byteCodeLength((const uint32_t*)code));

      std::memcpy(&m_dx9asm[0], code, m_dx9asm.size());
    }

    ~Direct3DShader9() {
      delete m_translation;
    }

    HRESULT STDMETHODCALLTYPE GetFunction(void* pShader, UINT* pSizeOfData) override {
      if (pSizeOfData == nullptr)
        return log::d3derr(D3DERR_INVALIDCALL, "GetFunction: pSizeOfData was nullptr.");

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

      if (riid == __uuidof(Base) || riid == __uuidof(IUnknown)) {
        *ppv = ref(this);
        return D3D_OK;
      }

      return E_NOINTERFACE;
    }

    const dx9asm::ShaderBytecode* GetTranslation() const {
      return m_translation;
    }

    D3D11Shader* GetD3D11Shader() {
      return m_shader.ptr();
    }

    void LinkInput(ID3D11InputLayout* inputLayout, IDirect3DVertexDeclaration9* vertDcl) {
      m_inputLinks.push_back(InputLink{ inputLayout, vertDcl });
    }

    ID3D11InputLayout* GetLinkedInput(IDirect3DVertexDeclaration9* vertDcl) {
      if (vertDcl == nullptr)
        return nullptr;

      for (InputLink& link : m_inputLinks) {
        if (link.vertexDcl == vertDcl)
          return link.inputLayout.ptr();
      }

      return nullptr;
    }

  private:

    const uint32_t m_shaderNum;
    std::vector<InputLink> m_inputLinks;
    std::vector<uint32_t> m_dx9asm;
    Com<D3D11Shader> m_shader;
    const dx9asm::ShaderBytecode* m_translation;
  };

  using Direct3DVertexShader9 = Direct3DShader9<ID3D11VertexShader, IDirect3DVertexShader9>;
  using Direct3DPixelShader9 = Direct3DShader9<ID3D11PixelShader, IDirect3DPixelShader9>;

}