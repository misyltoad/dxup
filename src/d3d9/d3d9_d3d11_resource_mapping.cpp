#include "d3d9_d3d11_resource.h"
#include "d3d9_format.h"

namespace dxup {

  bool DXUPResource::IsStagingRectDegenerate(UINT subresource) {
    return isRectDegenerate(m_stagingRects[subresource]);
  }

  UINT DXUPResource::CalcMapFlags(UINT d3d9LockFlags) {
    return d3d9LockFlags & D3DLOCK_DONOTWAIT ? D3D11_MAP_FLAG_DO_NOT_WAIT : 0;
  }

  D3D11_MAP DXUPResource::CalcMapType(UINT d3d9LockFlags, DWORD d3d9Usage) {
    if (m_dynamic) {
      if (d3d9LockFlags & D3DLOCK_NOOVERWRITE)
        return D3D11_MAP_WRITE_NO_OVERWRITE;

      if (d3d9LockFlags & D3DLOCK_DISCARD)
        return D3D11_MAP_WRITE_DISCARD;
    }

    if (d3d9LockFlags & D3DLOCK_READONLY)
      return D3D11_MAP_READ;

    if (d3d9Usage & D3DUSAGE_WRITEONLY)
      return D3D11_MAP_WRITE;

    return D3D11_MAP_READ_WRITE;
  }

  inline void FixRect(RECT& rect) {
    RECT newRect;
    newRect.left = std::min(rect.left, rect.right);
    newRect.right = std::max(rect.left, rect.right);

    newRect.top = std::min(rect.top, rect.bottom);
    newRect.bottom = std::max(rect.top, rect.bottom);

    rect = newRect;
  }

  HRESULT DXUPResource::D3D9LockRect(UINT slice, UINT mip, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags, DWORD Usage) {
    if (!pLockedRect)
      return D3DERR_INVALIDCALL;

    pLockedRect->pBits = nullptr;
    pLockedRect->Pitch = 0;

    UINT subresource = D3D11CalcSubresource(mip, slice, m_mips);

    if (pRect == nullptr)
      std::memset(&m_stagingRects[subresource], 0, sizeof(RECT));
    else
      m_stagingRects[subresource] = *pRect;

    FixRect(m_stagingRects[subresource]);

    D3D11_MAPPED_SUBRESOURCE res;
    HRESULT result = m_device->GetContext()->Map(GetMapping(), subresource, CalcMapType(Flags, Usage), CalcMapFlags(Flags), &res);

    if (result == DXGI_ERROR_WAS_STILL_DRAWING)
      return D3DERR_WASSTILLDRAWING;

    if (FAILED(result))
      return D3DERR_INVALIDCALL;

    SetMipMapped(slice, mip);

    size_t offset = 0;

    if (!IsStagingRectDegenerate(subresource))
      offset = (pRect->top * res.RowPitch) + (pRect->left * bitsPerPixel(m_dxgiFormat) / 8);

    uint8_t* data = (uint8_t*)res.pData;
    pLockedRect->pBits = &data[offset];
    pLockedRect->Pitch = res.RowPitch;

    return D3D_OK;
  }

  HRESULT DXUPResource::D3D9UnlockRect(UINT slice, UINT mip) {
    m_device->GetContext()->Unmap(GetMapping(), D3D11CalcSubresource(mip, slice, m_mips));
    SetMipUnmapped(slice, mip);

    if (HasStaging() && CanPushStaging())
      PushStaging();

    return D3D_OK;
  }

  void DXUPResource::PushStaging() {
    for (uint32_t slice = 0; slice < m_slices; slice++) {
      uint64_t delta = GetChangedMips(slice);

      for (uint64_t mip = 0; mip < m_mips; mip++) {
        UINT subresource = D3D11CalcSubresource(mip, slice, m_mips);

        bool useRect = !IsStagingRectDegenerate(subresource);

        D3D11_BOX box = { 0 };
        if (useRect) {
          box.left = alignRectForFormat(true, m_dxgiFormat, m_stagingRects[subresource].left);
          box.top = alignRectForFormat(true, m_dxgiFormat, m_stagingRects[subresource].top);
          box.right = alignRectForFormat(false, m_dxgiFormat, m_stagingRects[subresource].right);
          box.bottom = alignRectForFormat(false, m_dxgiFormat, m_stagingRects[subresource].bottom);

          box.front = 0;
          box.back = 1;
        }

        if (delta & (1ull << mip))
          m_device->GetContext()->CopySubresourceRegion(GetResource(), subresource, box.left, box.top, 0, GetStaging(), subresource, useRect ? &box : nullptr);
      }
    }
    
    ResetMipMapTracking();
  }

}