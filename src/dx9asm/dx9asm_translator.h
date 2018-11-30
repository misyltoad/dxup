#pragma once

#include "dx9asm_meta.h"
#include "dx9asm_util.h"
#include "dxbc_helpers.h"
#include <stdint.h>
#include <algorithm>
#include <vector>

namespace dxapex {

  namespace dx9asm {
    
    class ITranslatedShaderDXBC : public IUnknown {

    public:

      virtual void GetDXBCBytecode(uint32_t* code, uint32_t* size) = 0;
      virtual uint32_t GetConstantByteOffset(uint32_t constant) = 0;
      virtual ShaderType GetShaderType() = 0;

    };

    void toDXBC(const uint32_t* dx9asm, ITranslatedShaderDXBC** dxbc);

    struct Vec4Type {
      uint32_t valueTokens[4];
    };

    struct RegisterMapping {
      uint32_t dx9Type;
      uint32_t dx9Id;
      DXBCOperand dxbcOperand;
    };

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

      inline void push(uint32_t token) {
        m_dxbcCode.push_back(token);
      }

      const RegisterMapping* getRegisterMapping(uint32_t type, uint32_t index) const {
        for (const RegisterMapping& mapping : m_registerMap) {
          if (mapping.dx9Type == type && mapping.dx9Id == index)
            return &mapping;
        }
        return nullptr;
      }

      void addRegisterMapping(bool generateDXBCId, RegisterMapping& mapping) {
        DXBCOperand& dxbcOperand = mapping.dxbcOperand;
        uint32_t dataIndex = dxbcOperand.m_dataCount - 1;
        if (generateDXBCId) {
          uint32_t highestIdForType = 0;
          for (const RegisterMapping& lum : m_registerMap) {
            const DXBCOperand& lumOperand = lum.dxbcOperand;
            if (lumOperand.m_registerType == dxbcOperand.m_registerType && dxbcOperand.m_dataCount < 4)
              highestIdForType = max(highestIdForType, lumOperand.m_data[lumOperand.m_dataCount - 1]);
          }

          highestIdForType++;
          dxbcOperand.m_data[dataIndex] = highestIdForType;
        }

        m_registerMap.push_back(mapping);
      }

      bool handleOperation(uint32_t token);

      bool translate();


      bool handleComment(DX9Operation& operation);
      bool handleDef(DX9Operation& operation);

      inline std::vector<uint32_t>& getCode() {
        return m_dxbcCode;
      }

    private:
      bool handleUniqueOperation(DX9Operation& operation);

      bool handleStandardOperation(DX9Operation& operation);

      const uint32_t* m_base = nullptr;
      const uint32_t* m_head = nullptr;
      const CTHeader* m_ctab = nullptr;

      std::vector<uint32_t> m_dxbcCode;

      std::vector<RegisterMapping> m_registerMap;
    };

  }

}