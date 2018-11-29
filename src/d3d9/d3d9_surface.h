#pragma once

#include "d3d9_resource.h"
#include <vector>

namespace dxapex {

  using Direct3DSurface9Base = Direct3DResource9<D3DRTYPE_SURFACE, IDirect3DSurface9>;

  // A d3d9 surface is essentially some subresource of a d3d11 texture.
  class Direct3DSurface9 final : public Direct3DSurface9Base {
    
  public:

    Direct3DSurface9(bool depthStencil, UINT subresource, Direct3DDevice9Ex* device, IUnknown* container, ID3D11Texture2D* texture, D3DPOOL pool, DWORD usage);

    HRESULT WINAPI QueryInterface(REFIID riid, LPVOID* ppv) override;
    HRESULT WINAPI GetContainer(REFIID riid, void** ppContainer) override;
    HRESULT WINAPI GetDesc(D3DSURFACE_DESC *pDesc) override;
    HRESULT WINAPI LockRect(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) override;
    HRESULT WINAPI UnlockRect() override;
    HRESULT WINAPI GetDC(HDC *phdc) override;
    HRESULT WINAPI ReleaseDC(HDC hdc) override;

    void GetD3D11Texture(ID3D11Texture2D** texture);
    void GetDXGISurface(IDXGISurface1** surface);
    void GetD3D9Texture(Direct3DTexture9** texture);
    void GetD3D11RenderTarget(ID3D11RenderTargetView** rtv);
    UINT GetSubresource();

  private:

    Com<IUnknown> m_container;
    Com<ID3D11Texture2D> m_d3d11texture;
    Com<IDXGISurface1> m_surface;
    Com<ID3D11RenderTargetView> m_rtView;

    UINT m_subresource;
    std::vector<uint8_t> m_surfaceData;
    UINT m_surfaceDataPitch;
    void* m_resourceMapPtr;
  };

}