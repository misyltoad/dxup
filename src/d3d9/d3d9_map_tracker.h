#pragma once

namespace dxup {

  class D3D9MapTracker {

  public:

    D3D9MapTracker(uint32_t slices)
      : m_slices{ slices } {
      ResetMipMapTracking();
    }

    inline UINT GetSlices() {
      return m_slices;
    }

    inline void SetMipMapped(UINT slice, UINT mip) {
      m_mappedSubresources[slice] |= 1 << mip;
    }
    inline void SetMipUnmapped(UINT slice, UINT mip) {
      m_unmappedSubresources[slice] |= 1 << mip;
    }
    inline uint64_t GetChangedMips(UINT slice) {
      return m_mappedSubresources[slice];
    }
    inline bool CanPushStaging() {
      for (uint32_t i = 0; i < GetSlices(); i++) {
        if ((m_mappedSubresources[i] - m_unmappedSubresources[i]) != 0)
          return false;
      }
      return true;
    }
    inline void ResetMipMapTracking() {
      for (uint32_t slice = 0; slice < GetSlices(); slice++) {
        m_mappedSubresources[slice] = 0;
        m_unmappedSubresources[slice] = 0;
      }
    }

  private:

    uint32_t m_slices;

    uint64_t m_mappedSubresources[6];
    uint64_t m_unmappedSubresources[6];
  };

}