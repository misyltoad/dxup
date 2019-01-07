#pragma once
#include "dx9asm_register_mapping.h"
#include "dx9asm_meta.h"
#include <unordered_map>

namespace dxup {

  namespace dx9asm {

    class ShaderCodeTranslator;

    struct TransientRegisterMapping {
      uint32_t dxbcRegNum;

      uint32_t d3d9UsageIndex;
      uint32_t d3d9Usage;

      const char* dxbcSemanticName;
      uint32_t dxbcSemanticIndex;
    };

    class RegisterMap {
      using RegisterMapInternal = std::unordered_map<RegisterId, RegisterMapping, RegisterIdHasher, RegisterIdEqual>;

    public:

      inline void reset() {
        m_map.clear();
        m_highestInternalTemp = UINT32_MAX;
      }

      inline const RegisterMapping* getRegisterMapping(RegisterId id) const {
        auto iter = m_map.find(id);
        if (iter == m_map.end())
          return nullptr;

        return &iter->second;
      }

      inline RegisterMapping* getRegisterMapping(RegisterId id) {
        return const_cast<RegisterMapping*>(std::as_const(*this).getRegisterMapping(id));
      }

      inline uint32_t getHighestIdForDXBCType(uint32_t type) const {
        const uint32_t invalidId = UINT32_MAX;

        uint32_t highestIdForType = invalidId;

        for (auto iter : m_map) {
          const DXBCOperand& lumOperand = iter.second.getDXBCOperand();

          if (lumOperand.getRegisterType() == type) {
            highestIdForType = std::max(highestIdForType, lumOperand.getRegNumber());

            if (highestIdForType == invalidId)
              highestIdForType = lumOperand.getRegNumber();
          }
        }

        return highestIdForType;
      }

      static std::vector<TransientRegisterMapping>& getTransientMappings();

      inline uint32_t getDXBCTypeCount(uint32_t type) const {
        uint32_t highestId = getHighestIdForDXBCType(type);
        if (highestId == UINT32_MAX)
          return 0;

        return highestId + 1;
      }

      RegisterMapping* lookupOrCreateRegisterMapping(const ShaderCodeTranslator& translator, RegisterId id, uint32_t mask, uint32_t versionOverride = 0);

      RegisterMapping* lookupOrCreateRegisterMapping(const ShaderCodeTranslator& translator, const DX9Operand& operand, uint32_t regOffset = 0);

      uint32_t getTransientId(const DclInfo& info);

      inline void generateId(bool transient, DXBCOperand& operand, const DclInfo& info) {
        uint32_t& regNumber = operand.getRegNumber();

        if (!transient) {
          uint32_t highestIdForType = getHighestIdForDXBCType(operand.getRegisterType());

          highestIdForType++;
          regNumber = highestIdForType;
        }
        else
          regNumber = getTransientId(info);
      }

      inline void addRegisterMapping(RegisterId id, RegisterMapping& mapping) {
        m_map[id] = mapping;
      }

      inline uint32_t getTotalTempCount() {
        uint32_t highestRealTempId = getHighestIdForDXBCType(D3D10_SB_OPERAND_TYPE_TEMP);
        if (highestRealTempId == UINT32_MAX && m_highestInternalTemp == UINT32_MAX)
          return 0;

        if (highestRealTempId == UINT32_MAX)
          return m_highestInternalTemp + 1;

        if (m_highestInternalTemp == UINT32_MAX)
          return highestRealTempId + 1;

        return std::max(highestRealTempId, m_highestInternalTemp) + 1;
      }

      inline DXBCOperand getNextInternalTemp() {
        m_highestInternalTemp = getDXBCTypeCount(D3D10_SB_OPERAND_TYPE_TEMP);

        DXBCOperand op{ D3D10_SB_OPERAND_TYPE_TEMP, 1 };
        op.stripModifier();
        op.setRepresentation(0, D3D10_SB_OPERAND_INDEX_IMMEDIATE32);
        op.setData(&m_highestInternalTemp, 1);

        return op;
      }

      inline RegisterMapInternal& getRegisterMappings() {
        return m_map;
      }
    private:

      uint32_t m_highestInternalTemp = UINT32_MAX;
      RegisterMapInternal m_map;
    };

  }

}