#include "d3d9_d3d11_resource.h"
#include "d3d9_format.h"

namespace dxup {

  bool DXUPResource::IsStagingBoxDegenerate(UINT subresource) {
    return isBoxDegenerate(m_stagingBoxes[subresource]);
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

  void DXUPResource::MarkDirty(UINT slice, UINT mip) {
    m_dirtySubresources[slice] = 1ull << mip;
  }
  void DXUPResource::MakeClean() {
    bool dirty = false;
    for (uint32_t slice = 0; slice < m_slices; slice++) {
      if (m_dirtySubresources[slice] != 0)
        dirty = true;
    }

    if (!dirty)
      return;

    // This is needed for RTs, we only want staging buffers for them if they NEED them.
    if (GetStaging() == nullptr) {
      D3D11_TEXTURE2D_DESC desc;
      ID3D11Texture2D* tex = GetResourceAs<ID3D11Texture2D>();
      tex->GetDesc(&desc);

      makeStagingDesc(desc, 0, D3DFMT_UNKNOWN);
      m_device->GetD3D11Device()->CreateTexture2D(&desc, nullptr, (ID3D11Texture2D**)&m_staging);
    }

    for (uint32_t slice = 0; slice < m_slices; slice++) {
      uint64_t* dirtyFlags = &m_dirtySubresources[slice];

      for (uint64_t mip = 0; mip < m_mips; mip++) {
        UINT subresource = D3D11CalcSubresource(mip, slice, m_mips);
        if (*dirtyFlags & (1ull << mip))
          m_device->GetContext()->CopySubresourceRegion(GetStaging(), subresource, 0, 0, 0, GetResource(), subresource, nullptr);
      }

      *dirtyFlags = 0;
    }
  }

  HRESULT DXUPResource::D3D9LockBox(UINT slice, UINT mip, D3DLOCKED_BOX* pLockedBox, CONST D3DBOX* pBox, DWORD Flags, DWORD Usage) {
    CriticalSection cs(m_device);

    if (pLockedBox == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "D3D9LockBox: return value (locked box) was nullptr.");

    pLockedBox->pBits = nullptr;
    pLockedBox->RowPitch = 0;
    pLockedBox->SlicePitch = 0;

    UINT subresource = D3D11CalcSubresource(mip, slice, m_mips);

    if (pBox == nullptr)
      std::memset(&m_stagingBoxes[subresource], 0, sizeof(D3DBOX));
    else
      m_stagingBoxes[subresource] = *pBox;

    if (!(Flags & D3DLOCK_DISCARD) && !(Flags & D3DLOCK_NOOVERWRITE) && !(Usage & D3DUSAGE_WRITEONLY))
      MakeClean();

    D3D11_MAPPED_SUBRESOURCE res;
    HRESULT result = m_device->GetContext()->Map(GetMapping(), subresource, CalcMapType(Flags, Usage), CalcMapFlags(Flags), &res);

    if (result == DXGI_ERROR_WAS_STILL_DRAWING)
      return D3DERR_WASSTILLDRAWING;

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "D3D9LockRect: unknown error mapping subresource.");

    SetMipMapped(slice, mip);

    size_t offset = 0;

    if (!IsStagingBoxDegenerate(subresource))
      offset = (m_stagingBoxes[subresource].Top * res.RowPitch) + (m_stagingBoxes[subresource].Left * bitsPerPixel(m_dxgiFormat) / 8);

    uint8_t* data = (uint8_t*)res.pData;
    pLockedBox->pBits = &data[offset];
    pLockedBox->RowPitch = res.RowPitch;
    pLockedBox->SlicePitch = res.DepthPitch;

    return D3D_OK;
  }

  HRESULT DXUPResource::D3D9UnlockBox(UINT slice, UINT mip) {
    CriticalSection cs(m_device);

    UINT subresource = D3D11CalcSubresource(mip, slice, m_mips);

    ID3D11DeviceContext* context = m_device->GetContext();
    context->Unmap(GetMapping(), D3D11CalcSubresource(mip, slice, m_mips));
    SetMipUnmapped(slice, mip);

    // We need to make this format an 8888. DXGI has no 888 type.
    if (m_fixup8888 != nullptr) {
      D3D11_MAPPED_SUBRESOURCE d3d9Res;
      D3D11_MAPPED_SUBRESOURCE fixupRes;
      context->Map(GetStaging(), subresource, D3D11_MAP_READ, 0, &d3d9Res);
      context->Map(m_fixup8888.ptr(), subresource, D3D11_MAP_WRITE, 0, &fixupRes);

      D3D11_TEXTURE2D_DESC desc;
      ID3D11Texture2D* texture = reinterpret_cast<ID3D11Texture2D*>(m_staging.ptr());
      texture->GetDesc(&desc);

      uint8_t* read = (uint8_t*)d3d9Res.pData;
      uint8_t* write = (uint8_t*)fixupRes.pData;

      // TODO: Investigate if this is right or good enough.
      uint32_t mippedHeight = std::max(1u, desc.Height >> mip);
      uint32_t mippedWidth = std::max(1u, desc.Width >> mip);

      for (uint32_t y = 0; y < mippedHeight; y++) {
        for (uint32_t x = 0; x < mippedWidth; x++) {
          for (uint32_t c = 0; c < 3; c++) {
            write[y * fixupRes.RowPitch + x * 4 + c] = read[y * fixupRes.RowPitch + x * 3 + c];
          }
          write[y * fixupRes.RowPitch + x * 4 + 3] = 255;
        }
      }

      context->Unmap(m_fixup8888.ptr(), subresource);
      context->Unmap(GetStaging(), subresource);
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

        bool useRect = !IsStagingBoxDegenerate(subresource);

        D3D11_BOX box = { 0 };
        if (useRect) {
          D3D11_RESOURCE_DIMENSION dimension;
          GetResource()->GetType(&dimension);

          if (dimension == D3D11_RESOURCE_DIMENSION_BUFFER) {
            box.top = 0;
            box.bottom = 1;
            box.left = m_stagingBoxes[subresource].Left;
            box.right = m_stagingBoxes[subresource].Right;
          }
          else {
            box.top = alignRectForFormat(true, m_dxgiFormat, m_stagingBoxes[subresource].Top);
            box.bottom = alignRectForFormat(false, m_dxgiFormat, m_stagingBoxes[subresource].Bottom);
            box.left = alignRectForFormat(true, m_dxgiFormat, m_stagingBoxes[subresource].Left);
            box.right = alignRectForFormat(false, m_dxgiFormat, m_stagingBoxes[subresource].Right);
          }

          if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D || dimension == D3D11_RESOURCE_DIMENSION_BUFFER) {
            box.front = 0;
            box.back = 1;
          }
          else {
            box.front = alignRectForFormat(true, m_dxgiFormat, m_stagingBoxes[subresource].Front);
            box.back = alignRectForFormat(false, m_dxgiFormat, m_stagingBoxes[subresource].Back);
          }
        }

        if (delta & (1ull << mip))
          m_device->GetContext()->CopySubresourceRegion(GetResource(), subresource, box.left, box.top, 0, m_fixup8888 == nullptr ? GetStaging() : m_fixup8888.ptr(), subresource, useRect ? &box : nullptr);
      }
    }
    
    ResetMipMapTracking();
  }

}