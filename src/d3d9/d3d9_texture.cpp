#include "d3d9_texture.h"
#include "d3d9_device.h"

namespace dxup {

  // Direct3DTexture9

  Direct3DTexture9::Direct3DTexture9(Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& desc)
    : Direct3DTexture9Base{ device, resource, desc } {
    m_surfaces.reserve(resource->GetSubresources());

    for (UINT mip = 0; mip < resource->GetMips(); mip++) {
      Direct3DSurface9* surface = Direct3DSurface9::Wrap(0, mip, device, this, resource, desc);
      surface->AddRefPrivate();

      m_surfaces.push_back(surface);
    }
  }

  Direct3DTexture9::~Direct3DTexture9() {
    for (IDirect3DSurface9* surface : m_surfaces) {
      Direct3DSurface9* internalSurface = reinterpret_cast<Direct3DSurface9*>(surface);
      internalSurface->ReleasePrivate();
    }
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
    CriticalSection(this->GetD3D9Device());

    if (Level >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetLevelDesc: level out of bounds.");

    return m_surfaces[Level]->GetDesc(pDesc);
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::GetSurfaceLevel(UINT Level, IDirect3DSurface9** ppSurfaceLevel) {
    CriticalSection(this->GetD3D9Device());

    InitReturnPtr(ppSurfaceLevel);

    if (Level >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetSurfaceLevel: subresource out of bounds (Level = %d).", Level);

    if (ppSurfaceLevel == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetSurfaceLevel: ppSurfaceLevel was nullptr.");

    *ppSurfaceLevel = ref(m_surfaces[Level]);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags) {
    CriticalSection(this->GetD3D9Device());

    if (Level >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "LockRect: subresource out of bounds (Level = %d).", Level);

    return m_surfaces[Level]->LockRect(pLockedRect, pRect, Flags);
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::UnlockRect(UINT Level) {
    CriticalSection(this->GetD3D9Device());

    if (Level >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "UnlockRect: subresource out of bounds (Level = %d).", Level);

    return m_surfaces[Level]->UnlockRect();
  }

  HRESULT STDMETHODCALLTYPE Direct3DTexture9::AddDirtyRect(const RECT* pDirtyRect) {
    CriticalSection(this->GetD3D9Device());
    log::stub("Direct3DTexture9::AddDirtyRect");

    return D3D_OK;
  }

  HRESULT Direct3DTexture9::Create(Direct3DDevice9Ex* device,
                                   UINT width,
				                   UINT height,
                                   UINT levels,
                                   DWORD usage,
                                   D3DFORMAT format,
                                   D3DPOOL pool,
                                   Direct3DTexture9** outTexture) {
    InitReturnPtr(outTexture);

    if (width == 0)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DTexture9::Create: width was 0.");

    if (height == 0)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DTexture9::Create: height was 0.");

    if (usage & D3DUSAGE_AUTOGENMIPMAP && levels > 1)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DTexture9::Create: mipmap generation requested with more than 1 level.");

    if (!device->checkFormat(usage, D3DRTYPE_TEXTURE, format))
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DTexture9::Create: unsupported format (%d).", format);

    if (outTexture == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DTexture9::Create: outTexture was nullptr.");

    D3D11_USAGE d3d11Usage = convert::usage(pool, usage, D3DRTYPE_TEXTURE);

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width;
    desc.Height = height;
    desc.Format = convert::format(format);
    desc.Usage = d3d11Usage;
    desc.CPUAccessFlags = convert::cpuFlags(pool, usage, D3DRTYPE_TEXTURE);
    desc.MipLevels = d3d11Usage == D3D11_USAGE_DYNAMIC ? 1 : levels;
    desc.ArraySize = 1;

    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;

    if (!(usage & D3DUSAGE_DEPTHSTENCIL) && d3d11Usage != D3D11_USAGE_STAGING)
      desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

    if (d3d11Usage != D3D11_USAGE_STAGING) {
      desc.BindFlags |= ((usage & D3DUSAGE_RENDERTARGET) || (usage & D3DUSAGE_AUTOGENMIPMAP)) ? D3D11_BIND_RENDER_TARGET : 0;
      desc.BindFlags |= (usage & D3DUSAGE_DEPTHSTENCIL) ? D3D11_BIND_DEPTH_STENCIL : 0;

      desc.MiscFlags |= (usage & D3DUSAGE_AUTOGENMIPMAP) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
    }

    Com<ID3D11Texture2D> texture;
    HRESULT result = device->GetD3D11Device()->CreateTexture2D(&desc, nullptr, &texture);

    if (result == E_OUTOFMEMORY)
      return log::d3derr(D3DERR_OUTOFVIDEOMEMORY, "Direct3DTexture9::Create: out of vram.");

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DTexture9::Create: failed to create D3D11 texture. D3DFORMAT: %d, DXGI_FORMAT: %d", format, desc.Format); // TODO: stringify

    DXUPResource* resource = DXUPResource::Create(device, texture.ptr(), usage, format);
    if (resource == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DTexture9::Create: failed to create DXUP resource.");

    D3D9ResourceDesc d3d9Desc;
    d3d9Desc.Format = format;
    d3d9Desc.Pool = pool;
    d3d9Desc.Usage = usage;

    *outTexture = ref(new Direct3DTexture9(device, resource, d3d9Desc));

    return D3D_OK;
  }

  // Direct3DCubeTexture9

  Direct3DCubeTexture9::Direct3DCubeTexture9(Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& desc)
    : Direct3DCubeTexture9Base{ device, resource, desc } {
    m_surfaces.reserve(resource->GetSubresources());

    // This should allow us to match D3D11CalcSubresource.
    for (UINT slice = 0; slice < 6; slice++) {
      for (UINT mip = 0; mip < resource->GetMips(); mip++) {
        Direct3DSurface9* surface = Direct3DSurface9::Wrap(slice, mip, device, this, resource, desc);
        surface->AddRefPrivate();

        m_surfaces.push_back(surface);
      }
    }
  }

  Direct3DCubeTexture9::~Direct3DCubeTexture9() {
    for (IDirect3DSurface9* surface : m_surfaces) {
      Direct3DSurface9* internalSurface = reinterpret_cast<Direct3DSurface9*>(surface);
      internalSurface->ReleasePrivate();
    }
  }

  HRESULT STDMETHODCALLTYPE Direct3DCubeTexture9::GetLevelDesc(UINT Level, D3DSURFACE_DESC *pDesc) {
    CriticalSection(this->GetD3D9Device());

    if (Level >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetLevelDesc: level out of bounds.");

    return m_surfaces[Level]->GetDesc(pDesc);
  }

  HRESULT STDMETHODCALLTYPE Direct3DCubeTexture9::GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface) {
    CriticalSection(this->GetD3D9Device());

    UINT subresource = D3D11CalcSubresource(Level, (UINT)FaceType, this->GetDXUPResource()->GetMips());

    InitReturnPtr(ppCubeMapSurface);

    if (subresource >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetCubeMapSurface: subresource out of bounds (FaceType = %d, Level = %d).", FaceType, Level);

    if (ppCubeMapSurface == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetCubeMapSurface: ppCubeMapSurface was nullptr.");

    *ppCubeMapSurface = ref(m_surfaces[subresource]);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DCubeTexture9::LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags) {
    CriticalSection(this->GetD3D9Device());

    UINT subresource = D3D11CalcSubresource(Level, FaceType, this->GetDXUPResource()->GetMips());

    if (subresource >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "LockRect: subresource out of bounds (Level = %d).", Level);

    return m_surfaces[subresource]->LockRect(pLockedRect, pRect, Flags);
  }

  HRESULT STDMETHODCALLTYPE Direct3DCubeTexture9::UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level) {
    CriticalSection(this->GetD3D9Device());

    UINT subresource = D3D11CalcSubresource(Level, (UINT)FaceType, this->GetDXUPResource()->GetMips());

    if (subresource >= m_surfaces.size())
      return log::d3derr(D3DERR_INVALIDCALL, "UnlockRect: subresource out of bounds (FaceType = %d, Level = %d).", FaceType, Level);

    return m_surfaces[subresource]->UnlockRect();
  }

  HRESULT STDMETHODCALLTYPE Direct3DCubeTexture9::AddDirtyRect(D3DCUBEMAP_FACES FaceType, const RECT* pDirtyRect) {
    CriticalSection(this->GetD3D9Device());
    log::stub("Direct3DCubeTexture9::AddDirtyRect");

    return D3D_OK;
  }

  HRESULT Direct3DCubeTexture9::Create(Direct3DDevice9Ex* device,
                                       UINT edgeLength,
                                       UINT levels,
                                       DWORD usage,
                                       D3DFORMAT format,
                                       D3DPOOL pool,
                                       Direct3DCubeTexture9** outTexture) {
    InitReturnPtr(outTexture);

    if (edgeLength == 0)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DCubeTexture9::Create: edgeLength was 0.");

    if (usage& D3DUSAGE_AUTOGENMIPMAP&& levels > 1)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DCubeTexture9::Create: mipmap generation requested with more than 1 level.");

    if (!device->checkFormat(usage, D3DRTYPE_CUBETEXTURE, format))
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DCubeTexture9::Create: unsupported format (%d).", format);

    if (outTexture == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DCubeTexture9::Create: outTexture was nullptr.");

    D3D11_USAGE d3d11Usage = convert::usage(pool, usage, D3DRTYPE_CUBETEXTURE);

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = edgeLength;
    desc.Height = edgeLength;
    desc.Format = convert::format(format);
    desc.Usage = d3d11Usage;
    desc.CPUAccessFlags = convert::cpuFlags(pool, usage, D3DRTYPE_CUBETEXTURE);
    desc.MipLevels = levels;
    desc.ArraySize = 6;

    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;

    if (d3d11Usage != D3D11_USAGE_STAGING) {
      desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

      // Todo! Investigate below flags:
      desc.BindFlags |= ((usage & D3DUSAGE_RENDERTARGET) || (usage & D3DUSAGE_AUTOGENMIPMAP)) ? D3D11_BIND_RENDER_TARGET : 0;
      desc.MiscFlags |= (usage & D3DUSAGE_AUTOGENMIPMAP) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
    }

    Com<ID3D11Texture2D> texture;
    HRESULT result = device->GetD3D11Device()->CreateTexture2D(&desc, nullptr, &texture);

    if (result == E_OUTOFMEMORY)
      return log::d3derr(D3DERR_OUTOFVIDEOMEMORY, "Direct3DCubeTexture9::Create: out of vram.");

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DCubeTexture9::Create: failed to create D3D11 texture. D3DFORMAT: %d, DXGI_FORMAT: %d", format, desc.Format); // TODO: stringify

    DXUPResource * resource = DXUPResource::Create(device, texture.ptr(), usage, format);
    if (resource == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DCubeTexture9::Create: failed to create DXUP resource.");

    D3D9ResourceDesc d3d9Desc;
    d3d9Desc.Format = format;
    d3d9Desc.Pool = pool;
    d3d9Desc.Usage = usage;

    *outTexture = ref(new Direct3DCubeTexture9(device, resource, d3d9Desc));

    return D3D_OK;
  }

  // Direct3DVolumeTexture9

  Direct3DVolumeTexture9::Direct3DVolumeTexture9(Direct3DDevice9Ex* device, DXUPResource* resource, const D3D9ResourceDesc& desc)
    : Direct3DVolumeTexture9Base{ device, resource, desc } {
    m_volumes.reserve(resource->GetSubresources());

    for (UINT mip = 0; mip < resource->GetMips(); mip++) {
      Direct3DVolume9* volume = Direct3DVolume9::Wrap(mip, device, this, resource, desc);
      volume->AddRefPrivate();

      m_volumes.push_back(volume);
    }
  }

  Direct3DVolumeTexture9::~Direct3DVolumeTexture9() {
    for (IDirect3DVolume9* surface : m_volumes) {
      Direct3DVolume9* internalVolume = reinterpret_cast<Direct3DVolume9*>(surface);
      internalVolume->ReleasePrivate();
    }
  }

  HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture9::GetLevelDesc(UINT Level, D3DVOLUME_DESC* pDesc) {
    CriticalSection(this->GetD3D9Device());

    if (Level >= m_volumes.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetLevelDesc: level out of bounds.");

    return m_volumes[Level]->GetDesc(pDesc);
  }

  HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture9::GetVolumeLevel(UINT Level, IDirect3DVolume9** ppVolumeLevel) {
    CriticalSection(this->GetD3D9Device());

    InitReturnPtr(ppVolumeLevel);

    if (Level >= m_volumes.size())
      return log::d3derr(D3DERR_INVALIDCALL, "GetVolumeLevel: subresource out of bounds (Level = %d).", Level);

    if (ppVolumeLevel == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetVolumeLevel: ppVolumeLevel was nullptr.");

    *ppVolumeLevel = ref(m_volumes[Level]);

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture9::LockBox(UINT Level, D3DLOCKED_BOX* pLockedBox, const D3DBOX* pBox, DWORD Flags) {
    CriticalSection(this->GetD3D9Device());

    if (Level >= m_volumes.size())
      return log::d3derr(D3DERR_INVALIDCALL, "LockBox: subresource out of bounds (Level = %d).", Level);

    return m_volumes[Level]->LockBox(pLockedBox, pBox, Flags);
  }

  HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture9::UnlockBox(UINT Level) {
    CriticalSection(this->GetD3D9Device());

    if (Level >= m_volumes.size())
      return log::d3derr(D3DERR_INVALIDCALL, "UnlockBox: subresource out of bounds (Level = %d).", Level);

    return m_volumes[Level]->UnlockBox();
  }

  HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture9::AddDirtyBox(const D3DBOX* pDirtyBox) {
    CriticalSection(this->GetD3D9Device());
    log::stub("Direct3DVolumeTexture9::AddDirtyRect");

    return D3D_OK;
  }

  HRESULT Direct3DVolumeTexture9::Create(Direct3DDevice9Ex* device,
                                         UINT width,
                                         UINT height,
                                         UINT depth,
                                         UINT levels,
                                         DWORD usage,
                                         D3DFORMAT format,
                                         D3DPOOL pool,
                                         Direct3DVolumeTexture9** outTexture) {
    InitReturnPtr(outTexture);

    if (width == 0)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DVolumeTexture9::Create: width was 0.");

    if (height == 0)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DVolumeTexture9::Create: height was 0.");

    if (depth == 0)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DVolumeTexture9::Create: depth was 0.");

    if (!device->checkFormat(usage, D3DRTYPE_VOLUMETEXTURE, format))
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DVolumeTexture9::Create: unsupported format (%d).", format);

    if (outTexture == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DVolumeTexture9::Create: outTexture was nullptr.");

    D3D11_USAGE d3d11Usage = convert::usage(pool, usage, D3DRTYPE_VOLUMETEXTURE);

    D3D11_TEXTURE3D_DESC desc;
    desc.Width = width;
    desc.Height = height;
    desc.Depth = depth;
    desc.Format = convert::format(format);
    desc.Usage = d3d11Usage;
    desc.CPUAccessFlags = convert::cpuFlags(pool, usage, D3DRTYPE_VOLUMETEXTURE);
    desc.MipLevels = d3d11Usage == D3D11_USAGE_DYNAMIC ? 1 : levels;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;

    if (d3d11Usage != D3D11_USAGE_STAGING)
      desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

    Com<ID3D11Texture3D> texture;
    HRESULT result = device->GetD3D11Device()->CreateTexture3D(&desc, nullptr, &texture);

    if (result == E_OUTOFMEMORY)
      return log::d3derr(D3DERR_OUTOFVIDEOMEMORY, "Direct3DVolumeTexture9::Create: out of vram.");

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DVolumeTexture9::Create: failed to create D3D11 texture. D3DFORMAT: %d, DXGI_FORMAT: %d", format, desc.Format); // TODO: stringify

    DXUPResource * resource = DXUPResource::Create(device, texture.ptr(), usage, format);
    if (resource == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "Direct3DVolumeTexture9::Create: failed to create DXUP resource.");

    D3D9ResourceDesc d3d9Desc;
    d3d9Desc.Format = format;
    d3d9Desc.Pool = pool;
    d3d9Desc.Usage = usage;

    *outTexture = ref(new Direct3DVolumeTexture9(device, resource, d3d9Desc));

    return D3D_OK;
  }

}