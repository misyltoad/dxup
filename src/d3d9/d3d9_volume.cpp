#include "d3d9_volume.h"
#include "d3d9_format.h"
#include "d3d9_texture.h"
#include "d3d9_format.h"
#include "../util/config.h"
#include <algorithm>

namespace dxup {

  Direct3DVolume9::Direct3DVolume9(UINT mip, Direct3DDevice9Ex* device, IUnknown* container, DXUPResource* resource, const D3D9ResourceDesc& desc)
    : Direct3DVolume9Base{ device, nullptr, desc }
    , m_container{ container }
    , m_mip{ mip } {
    this->SetResource(resource);
  }

  HRESULT Direct3DVolume9::GetContainer(REFIID riid, void** ppContainer) {
    InitReturnPtr(ppContainer);

    if (ppContainer == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetContainer: ppContainer was nullptr.");

    if (m_container == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetContainer: m_container was nullptr.");

    return FAILED(m_container->QueryInterface(riid, ppContainer)) ? D3DERR_INVALIDCALL : D3D_OK;
  }

  HRESULT Direct3DVolume9::QueryInterface(REFIID riid, void** ppvObj) {
    InitReturnPtr(ppvObj);

    if (ppvObj == nullptr)
      return E_POINTER;

    // This isn't a IDirect3DResource9. W h y?
    if (riid == __uuidof(IDirect3DVolume9) || /*riid == __uuidof(IDirect3DResource9) ||*/ riid == __uuidof(IUnknown)) {
      *ppvObj = ref(this);
      return D3D_OK;
    }

    return E_NOINTERFACE;
  }

  HRESULT Direct3DVolume9::GetDesc(D3DVOLUME_DESC *pDesc) {
    if (!pDesc)
      return log::d3derr(D3DERR_INVALIDCALL, "GetDesc: pDesc was nullptr.");

    D3D11_TEXTURE3D_DESC desc;
    m_resource->GetResourceAs<ID3D11Texture3D>()->GetDesc(&desc);

    pDesc->Format = m_d3d9Desc.Format;
    pDesc->Height = std::max(1u, desc.Height >> m_mip);
    pDesc->Width = std::max(1u, desc.Width >> m_mip);
    pDesc->Depth = std::max(1u, desc.Depth >> m_mip);
    pDesc->Pool = m_d3d9Desc.Pool;
    pDesc->Type = D3DRTYPE_VOLUME;
    pDesc->Usage = m_d3d9Desc.Usage;

    return D3D_OK;
  }
  HRESULT Direct3DVolume9::LockBox(D3DLOCKED_BOX* pLockedBox, CONST D3DBOX* pBox, DWORD Flags) {
    return m_resource->D3D9LockBox(0, m_mip, pLockedBox, pBox, Flags, m_d3d9Desc.Usage);
  }
  HRESULT Direct3DVolume9::UnlockBox() {
    return m_resource->D3D9UnlockBox(0, m_mip);
  }

  ULONG STDMETHODCALLTYPE Direct3DVolume9::AddRef() {
    if (m_container != nullptr)
      m_container->AddRef();

    return Direct3DVolume9Base::AddRef();
  }
  ULONG STDMETHODCALLTYPE Direct3DVolume9::Release() {
    if (m_container != nullptr)
      m_container->Release();

    return Direct3DVolume9Base::Release();
  }

  UINT Direct3DVolume9::GetMip() {
    return m_mip;
  }
  UINT Direct3DVolume9::GetSubresource() {
    return GetMip();
  }

  void Direct3DVolume9::ClearResource() {
    m_resource = nullptr;
  }
  void Direct3DVolume9::SetResource(DXUPResource* resource) {
    this->ClearResource();

    m_resource = resource;
  }

  Direct3DVolume9* Direct3DVolume9::Wrap(UINT mip, Direct3DDevice9Ex* device, IUnknown* container, DXUPResource* resource, const D3D9ResourceDesc& desc) {
    return new Direct3DVolume9(mip, device, container, resource, desc);
  }
}