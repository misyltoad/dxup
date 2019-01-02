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

    D3D11_MAPPED_SUBRESOURCE res;
    HRESULT result = m_device->GetContext()->Map(GetMapping(), subresource, CalcMapType(Flags, Usage), CalcMapFlags(Flags), &res);

    if (result == DXGI_ERROR_WAS_STILL_DRAWING)
      return D3DERR_WASSTILLDRAWING;

    if (FAILED(result))
      return D3DERR_INVALIDCALL;

    SetMipMapped(slice, mip);

    size_t offset = 0;

    if (!IsStagingRectDegenerate(subresource))
      offset = (m_stagingRects[subresource].top * res.RowPitch) + (m_stagingRects[subresource].left * bitsPerPixel(m_dxgiFormat) / 8);

    uint8_t* data = (uint8_t*)res.pData;
    pLockedRect->pBits = &data[offset];
    pLockedRect->Pitch = res.RowPitch;

    return D3D_OK;
  }

  HRESULT DXUPResource::D3D9UnlockRect(UINT slice, UINT mip) {
    UINT subresource = D3D11CalcSubresource(mip, slice, m_mips);

    m_device->GetContext()->Unmap(GetMapping(), D3D11CalcSubresource(mip, slice, m_mips));
    SetMipUnmapped(slice, mip);

    // We need to make this format an 8888. DXGI has no 888 type.
    if (m_fixup8888 != nullptr) {
      D3D11_MAPPED_SUBRESOURCE d3d9Res;
      D3D11_MAPPED_SUBRESOURCE fixupRes;
      m_device->GetContext()->Map(GetStaging(), subresource, D3D11_MAP_READ, 0, &d3d9Res);
      m_device->GetContext()->Map(m_fixup8888.ptr(), subresource, D3D11_MAP_WRITE, 0, &fixupRes);

      D3D11_TEXTURE2D_DESC desc;
      ID3D11Texture2D* texture = reinterpret_cast<ID3D11Texture2D*>(m_staging.ptr());
      texture->GetDesc(&desc);

      uint8_t* read = (uint8_t*)d3d9Res.pData;
      uint8_t* write = (uint8_t*)fixupRes.pData;

      // TODO: Investigate if this is right or good enough.
      uint32_t mippedHeight = desc.Height >> mip;
      uint32_t mippedWidth = desc.Width >> mip;

      for (uint32_t y = 0; y < mippedHeight; y++) {
        for (uint32_t x = 0; x < mippedWidth; x++) {
          for (uint32_t c = 0; c < 3; c++) {
            write[y * fixupRes.RowPitch + x * 4 + c] = read[y * fixupRes.RowPitch + x * 3 + c];
          }
          write[y * fixupRes.RowPitch + x * 4 + 3] = 255;
        }
      }

      m_device->GetContext()->Unmap(m_fixup8888.ptr(), subresource);
      m_device->GetContext()->Unmap(GetStaging(), subresource);
    }

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
          m_device->GetContext()->CopySubresourceRegion(GetResource(), subresource, box.left, box.top, 0, m_fixup8888 == nullptr ? GetStaging() : m_fixup8888.ptr(), subresource, useRect ? &box : nullptr);
      }
    }
    
    ResetMipMapTracking();
  }

}