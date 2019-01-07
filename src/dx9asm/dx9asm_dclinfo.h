#pragma once

namespace dxup {

  namespace dx9asm {

    namespace dclFlags {
      const uint32_t input = 1 << 0;
      const uint32_t output = 1 << 1;
      const uint32_t target = 1 << 2;
      const uint32_t centroid = 1 << 3;
    }

    class DclInfo {
    public:
      DclInfo()
        : m_usage{ D3DDECLUSAGE_POSITION }, m_usageIndex{ 0 }, m_flags{ 0 } {}

      DclInfo(D3DDECLUSAGE usage, uint32_t usageIndex, uint32_t flags)
        : m_usage{ usage }, m_usageIndex{ usageIndex }, m_flags{ flags } {}

      inline bool isInput() const {
        return m_flags & dclFlags::input;
      }

      inline bool isOutput() const {
        return m_flags & dclFlags::output;
      }

      inline bool isCentroid() const {
        return m_flags & dclFlags::centroid;
      }

      inline bool isTarget() const {
        return m_flags & dclFlags::target;
      }

      inline D3DDECLUSAGE getUsage() const {
        return m_usage;
      }

      inline uint32_t getUsageIndex() const {
        return m_usageIndex;
      }

      inline bool isValid() const {
        return m_flags & dclFlags::input || m_flags & dclFlags::output;
      }

    private:
      D3DDECLUSAGE m_usage;
      uint32_t m_usageIndex;
      uint32_t m_flags;
    };

  }

}