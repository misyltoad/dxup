#pragma once

#include "dxbc_helpers.h"
#include <d3d9types.h>
#include <optional>
#include "dx9asm_register_id.h"
#include "dx9asm_dclinfo.h"

namespace dxup {

  namespace dx9asm {

    class RegisterMapping {
    public:
      RegisterMapping() {}

      RegisterMapping(const DXBCOperand& operand, uint32_t mask, const DclInfo& dcl = {})
        : m_dxbcOperand{ operand }, m_mask{ mask }, m_dclInfo{ dcl } {}

      inline void addToMask(uint32_t mask) {
        m_mask |= mask;
      }

      inline const DXBCOperand& getDXBCOperand() const {
        return m_dxbcOperand;
      }

      inline uint32_t getMask() const {
        return m_mask;
      }

      inline const DclInfo& getDclInfo() const {
        return m_dclInfo;
      }

    private:
      DXBCOperand m_dxbcOperand;
      uint32_t m_mask;
      DclInfo m_dclInfo;
    };
  }

}