#include "d3d9_d3d11_resource.h"
#include "d3d9_util.h"
#include <algorithm>

namespace dxup {

  // If we are not a dynamic resource or we need read access on our dynamic, we have a staging buffer.
  // We then map this and CopySubresourceRegion on unmapping.
  // If we don't need the staging res. will be nullptr, some logic relies on this.
  bool DXUPResource::NeedsStaging(D3D11_USAGE d3d11Usage, DWORD d3d9Usage) {
    return d3d11Usage == D3D11_USAGE_DEFAULT || !(d3d9Usage & D3DUSAGE_WRITEONLY);
  }

  DXUPResource* DXUPResource::CreateTexture2D(Direct3DDevice9Ex* device, ID3D11Texture2D* texture, DWORD d3d9Usage) {
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    Com<ID3D11Texture2D> stagingTexture;
    if (NeedsStaging(desc.Usage, d3d9Usage)) {
      makeStaging(desc, d3d9Usage);

      device->GetD3D11Device()->CreateTexture2D(&desc, nullptr, &stagingTexture);
    }

    Com<ID3D11ShaderResourceView> srv;

    if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
      device->GetD3D11Device()->CreateShaderResourceView(texture, nullptr, &srv);

    return new DXUPResource(device, texture, stagingTexture.ptr(), srv.ptr(), desc.Format, desc.ArraySize, std::max(desc.MipLevels, 1u), desc.Usage == D3D11_USAGE_DYNAMIC);
  }

  DXUPResource* DXUPResource::CreateBuffer(Direct3DDevice9Ex* device, ID3D11Buffer* buffer, DWORD d3d9Usage) {
    D3D11_BUFFER_DESC desc;
    buffer->GetDesc(&desc);

    Com<ID3D11Buffer> stagingBuffer;
    if (NeedsStaging(desc.Usage, d3d9Usage)) {
      makeStaging(desc, d3d9Usage);

      device->GetD3D11Device()->CreateBuffer(&desc, nullptr, &stagingBuffer);
    }

    return new DXUPResource(device, buffer, stagingBuffer.ptr(), nullptr, DXGI_FORMAT_UNKNOWN, 1, 1, desc.Usage == D3D11_USAGE_DYNAMIC);
  }

  DXUPResource* DXUPResource::Create(Direct3DDevice9Ex* device, ID3D11Resource* resource, DWORD d3d9Usage) {
    D3D11_RESOURCE_DIMENSION dimension;
    resource->GetType(&dimension);

    if (dimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
      return CreateTexture2D(device, useAs<ID3D11Texture2D>(resource), d3d9Usage);
    else if (dimension == D3D11_RESOURCE_DIMENSION_BUFFER)
      return CreateBuffer(device, useAs<ID3D11Buffer>(resource), d3d9Usage);

    log::fail("Unable to create DXUP resource for unknown type.");
    return nullptr;
  }

  bool DXUPResource::HasStaging() {
    return GetStaging() != nullptr;
  }

  ID3D11Resource* DXUPResource::GetResource() {
    return GetResourceAs<ID3D11Resource>();
  }

  ID3D11Resource* DXUPResource::GetStaging() {
    return GetStagingAs<ID3D11Resource>();
  }

  ID3D11Resource* DXUPResource::GetMapping() {
    return GetMappingAs<ID3D11Resource>();
  }

  ID3D11ShaderResourceView* DXUPResource::GetSRV() {
    return m_srv.ptr();
  }

  UINT DXUPResource::GetSlices() {
    return m_slices;
  }
  UINT DXUPResource::GetMips() {
    return m_mips;
  }
  UINT DXUPResource::GetSubresources() {
    return m_slices * m_mips;
  }

  void DXUPResource::SetMipMapped(UINT slice, UINT mip) {
    m_mappedSubresources[slice] |= 1 << mip;
  }
  void DXUPResource::SetMipUnmapped(UINT slice, UINT mip) {
    m_unmappedSubresources[slice] |= 1 << mip;
  }

  uint64_t DXUPResource::GetChangedMips(UINT slice) {
    return m_mappedSubresources[slice];
  }

  bool DXUPResource::CanPushStaging() {
    for (uint32_t i = 0; i < GetSlices(); i++) {
      if ((m_mappedSubresources[i] - m_unmappedSubresources[i]) != 0)
        return false;
    }
    return true;
  }

  void DXUPResource::ResetMipMapTracking() {
    for (uint32_t slice = 0; slice < GetSlices(); slice++) {
      m_mappedSubresources[slice] = 0;
      m_unmappedSubresources[slice] = 0;
    }
  }

  DXUPResource::DXUPResource(Direct3DDevice9Ex* device, ID3D11Resource* resource, ID3D11Resource* staging, ID3D11ShaderResourceView* srv, DXGI_FORMAT dxgiFormat, UINT slices, UINT mips, bool dynamic)
    : m_device{ device }
    , m_resource{ resource }
    , m_staging{ staging }
    , m_srv{ srv }
    , m_slices{ slices }
    , m_mips{ mips }
    , m_dxgiFormat{ dxgiFormat }
    , m_dynamic{ dynamic } {
    ResetMipMapTracking();
  }
}