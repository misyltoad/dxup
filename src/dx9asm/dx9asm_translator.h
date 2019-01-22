#pragma once

#include "dx9asm_meta.h"
#include "dx9asm_util.h"
#include "dxbc_helpers.h"
#include <stdint.h>
#include <algorithm>
#include <vector>
#include "dxbc_bytecode.h"
#include "dx9asm_register_map.h"

namespace dxup {

  namespace dx9asm {

    void toDXBC(const uint32_t* dx9asm, ShaderBytecode** dxbc);

    class DX9Operation;

    struct SamplerDesc {
      uint32_t index;
      uint32_t dimension;
    };

    class ShaderCodeTranslator {

    public:

      inline void reset(const uint32_t* code) {
        m_dxbcCode.clear();
        m_samplers.clear();
        m_map.reset();
        m_indirectConstantUsed = false;

        m_ctab = nullptr;
        m_base = code;
        m_head = code;
      }

      inline uint32_t getHeaderToken() const {
        return *m_base;
      }

      inline uint32_t getMajorVersion() const {
        return D3DSHADER_VERSION_MAJOR(getHeaderToken());
      }

      inline uint32_t getMinorVersion() const {
        return D3DSHADER_VERSION_MINOR(getHeaderToken());
      }

      inline uint32_t nextToken() {
        return *m_head++;
      }

      inline void skipTokens(uint32_t count) {
        for (uint32_t i = 0; i < count; i++)
          nextToken();
      }

      inline ShaderType getShaderType() const {
        return (getHeaderToken() & 0xFFFF0000) == 0xFFFF0000 ? ShaderType::Pixel : ShaderType::Vertex;
      }

      inline bool isTransient(bool input) const {
        return (getShaderType() == ShaderType::Pixel && input) ||
               (getShaderType() == ShaderType::Vertex && !input);
      }

      inline bool shouldGenerateId(bool transient) const {
        return true;//!transient || (transient && getMajorVersion() != 3);
      }

      bool handleOperation(uint32_t token);

      inline bool isSamplerUsed(uint32_t i) {
        for (auto& desc : m_samplers) {
          if (desc.index == i)
            return true;
        }

        return false;
      }

      inline bool isAnySamplerUsed() {
        return !m_samplers.empty();
      }

      inline void markIndirect() {
        m_indirectConstantUsed = true;
      }

      inline bool isIndirectMarked() {
        return m_indirectConstantUsed;
      }

      inline SamplerDesc* getSampler(uint32_t i) {
        for (auto& desc : m_samplers) {
          if (desc.index == i)
            return &desc;
        }

        return nullptr;
      }

      bool translate();

      bool handleComment(DX9Operation& operation);
      bool handleDcl(DX9Operation& operation);
      bool handleTex(DX9Operation& operation);
      bool handleGenericTexReg2XX(DX9Operation& operation, uint32_t swizzle);
      bool handleTexReg2Ar(DX9Operation& operation);
      bool handleTexReg2Gb(DX9Operation& operation);
      bool handleLrp(DX9Operation& operation);
      bool handleMov(DX9Operation& operation);
      bool handleMova(DX9Operation& operation);
      bool handleCmp(DX9Operation& operation);
      bool handleSlt(DX9Operation& operation);
      bool handleSge(DX9Operation& operation);
      bool handleNrm(DX9Operation& operation);
      bool handlePow(DX9Operation& operation);
      bool handleGenericDef(DX9Operation& operation, bool boolean);
      bool handleDef(DX9Operation& operation);
      bool handleDefi(DX9Operation& operation);
      bool handleDefB(DX9Operation& operation);
      bool handleSinCos(DX9Operation& operation);
      bool handleDp2Add(DX9Operation& operation);

      bool handleIf(DX9Operation& operation);
      bool handleIfc(DX9Operation& operation);

      bool handleScomp(bool lt, DX9Operation& operation);

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
      bool m_indirectConstantUsed = false;

      RegisterMap m_map;

      std::vector<SamplerDesc> m_samplers;
      std::vector<uint32_t> m_dxbcCode;
    };

  }

}
