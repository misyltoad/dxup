#pragma once

namespace dxup {

  namespace dx9asm {

    class RegisterId {
    public:
      RegisterId(uint32_t regType, uint32_t regNum)
        : m_dx9RegType{ regType }, m_dx9RegNum{ regNum } {}

      inline uint32_t getRegType() const {
        return m_dx9RegType;
      }

      inline uint32_t getRegNum() const {
        return m_dx9RegNum;
      }

    private:
      uint32_t m_dx9RegType;
      uint32_t m_dx9RegNum;
    };

    struct RegisterIdHasher {
      std::size_t operator()(const RegisterId& k) const {
        uint64_t key = k.getRegType();
        key |= ((uint64_t)k.getRegNum()) << 32;

        std::hash<uint64_t> hash;

        return hash(key);
      }
    };

    struct RegisterIdEqual {
      bool operator()(const RegisterId& a, const RegisterId& b) const {
        return a.getRegNum() == b.getRegNum() && a.getRegType() == b.getRegType();
      }
    };

  }

}