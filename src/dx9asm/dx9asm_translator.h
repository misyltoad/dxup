#pragma once

#include "dx9asm_meta.h"
#include "dx9asm_util.h"
#include "dxbc_helpers.h"
#include <stdint.h>
#include <algorithm>
#include <vector>
#include "dxbc_bytecode.h"

namespace dxapex {

  namespace dx9asm {

    void toDXBC(const uint32_t* dx9asm, ShaderBytecode** dxbc);

    struct RegisterMapping {
      uint32_t dx9Type;
      uint32_t dx9Id;
      DXBCOperand dxbcOperand;

      struct {
        bool hasUsage;
        D3DDECLUSAGE usage;
        uint32_t usageIndex;
      } dclInfo;
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

      inline const RegisterMapping* getRegisterMapping(const DX9Operand& operand) const {
        return getRegisterMapping(operand);
      }

      inline RegisterMapping* getRegisterMapping(const DX9Operand& operand) {
        return getRegisterMapping(operand.getRegType(), operand.getRegNumber());
      }

      inline const RegisterMapping* getRegisterMapping(uint32_t type, uint32_t index) const {
        return getRegisterMapping(type, index);
      }

      inline RegisterMapping* getRegisterMapping(uint32_t type, uint32_t index) {
        for (RegisterMapping& mapping : m_registerMap) {
          if (mapping.dx9Type == type && mapping.dx9Id == index)
            return &mapping;
        }
        return nullptr;
      }

      inline int32_t getHighestIdForDXBCType(uint32_t type) {
        const uint32_t invalidId = UINT32_MAX;

        uint32_t highestIdForType = invalidId;

        for (const RegisterMapping& lum : m_registerMap) {
          const DXBCOperand& lumOperand = lum.dxbcOperand;

          if (lumOperand.getRegisterType() == type) {
            highestIdForType = max(highestIdForType, lumOperand.getRegNumber());

            if (highestIdForType == invalidId)
              highestIdForType = lumOperand.getRegNumber();
          }
        }

        return highestIdForType;
      }

      RegisterMapping* lookupOrCreateRegisterMapping(const DX9Operand& operand, uint32_t regOffset = 0);

      inline void addRegisterMapping(bool generateDXBCId, RegisterMapping& mapping) {
        DXBCOperand& dxbcOperand = mapping.dxbcOperand;
        if (generateDXBCId) {
          uint32_t& regNumber = dxbcOperand.getRegNumber();
          int32_t highestIdForType = getHighestIdForDXBCType(dxbcOperand.m_registerType);

          highestIdForType++;
          regNumber = highestIdForType;
        }

        m_registerMap.push_back(mapping);
      }

      inline std::vector<RegisterMapping>& getRegisterMappings() {
        return m_registerMap;
      }

      bool handleOperation(uint32_t token);

      bool translate();

      bool handleComment(DX9Operation& operation);
      bool handleDcl(DX9Operation& operation);
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