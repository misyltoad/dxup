#pragma once

#include "dx9asm_meta.h"
#include "dx9asm_util.h"
#include "dxbc_helpers.h"
#include <stdint.h>
#include <algorithm>
#include <vector>
#include "dxbc_bytecode.h"
#include "dx9asm_register_map.h"

namespace dxapex {

  namespace dx9asm {

    void toDXBC(const uint32_t* dx9asm, ShaderBytecode** dxbc);

    class DX9Operation;

    class ShaderCodeTranslator {

    public:

      ShaderCodeTranslator(const uint32_t* code)
        : m_base{code} 
        , m_head{ code } {}

      inline uint32_t getHeaderToken() {
        return *m_base;
      }

      inline uint32_t getMajorVersion() {
        return D3DSHADER_VERSION_MAJOR(getHeaderToken());
      }

      inline uint32_t getMinorVersion() {
        return D3DSHADER_VERSION_MINOR(getHeaderToken());
      }

      inline uint32_t nextToken() {
        return *m_head++;
      }

      inline void skipTokens(uint32_t count) {
        for (uint32_t i = 0; i < count; i++)
          nextToken();
      }

      inline ShaderType getShaderType() {
        return (getHeaderToken() & 0xFFFF0000) == 0xFFFF0000 ? ShaderType::Pixel : ShaderType::Vertex;
      }

      bool handleOperation(uint32_t token);

      inline bool isSamplerUsed(uint32_t i) {
        return m_samplerMask & (1u << i);
      }

      inline bool isAnySamplerUsed() {
        return m_samplerMask != 0;
      }

      bool translate();

      bool handleComment(DX9Operation& operation);
      bool handleDcl(DX9Operation& operation);
      bool handleTex(DX9Operation& operation);
      bool handleLit(DX9Operation& operation);
      bool handleMad(DX9Operation& operation);
      bool handleDef(DX9Operation& operation);

      inline std::vector<uint32_t>& getCode() {
        return m_dxbcCode;
      }

      inline RegisterMap& getRegisterMap() {
        return m_map;
      }

    private:
      bool handleUniqueOperation(DX9Operation& operation);

      bool handleStandardOperation(DX9Operation& operation);

      const uint32_t* m_base = nullptr;
      const uint32_t* m_head = nullptr;
      const CTHeader* m_ctab = nullptr;

      RegisterMap m_map;

      uint32_t m_samplerMask = 0;

      std::vector<uint32_t> m_dxbcCode;
    };

  }

}