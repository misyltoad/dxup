#pragma once
#include "dx9asm_register_mapping.h"
#include "dx9asm_meta.h"

namespace dxapex {

  namespace dx9asm {

    class RegisterMap {
    public:
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

      inline uint32_t getHighestIdForDXBCType(uint32_t type) const {
        const uint32_t invalidId = UINT32_MAX;

        uint32_t highestIdForType = invalidId;

        for (const RegisterMapping& lum : m_registerMap) {
          const DXBCOperand& lumOperand = lum.dxbcOperand;

          if (lumOperand.getRegisterType() == type) {
            highestIdForType = std::max(highestIdForType, lumOperand.getRegNumber());

            if (highestIdForType == invalidId)
              highestIdForType = lumOperand.getRegNumber();
          }
        }

        return highestIdForType;
      }

      inline uint32_t getDXBCTypeCount(uint32_t type) const {
        return getHighestIdForDXBCType(type) + 1;
      }

      RegisterMapping* lookupOrCreateRegisterMapping(ShaderType type, uint32_t minorVersion, uint32_t majorVersion, uint32_t regType, uint32_t regNum, uint32_t readMask, uint32_t writeMask);

      RegisterMapping* lookupOrCreateRegisterMapping(ShaderType type, uint32_t minorVersion, uint32_t majorVersion, const DX9Operand& operand, uint32_t regOffset = 0);

      uint32_t getTransientId(DclInfo& info);

      inline void addRegisterMapping(bool transient, bool generateDXBCId, RegisterMapping& mapping) {
        DXBCOperand& dxbcOperand = mapping.dxbcOperand;
        if (generateDXBCId) {
          uint32_t& regNumber = dxbcOperand.getRegNumber();

          if (!transient) {
            uint32_t highestIdForType = getHighestIdForDXBCType(dxbcOperand.getRegisterType());

            highestIdForType++;
            regNumber = highestIdForType;
          }
          else
            regNumber = getTransientId(mapping.dclInfo);
        }

        m_registerMap.push_back(mapping);
      }

      inline std::vector<RegisterMapping>& getRegisterMappings() {
        return m_registerMap;
      }
    private:
      std::vector<RegisterMapping> m_registerMap;
    };

  }

}