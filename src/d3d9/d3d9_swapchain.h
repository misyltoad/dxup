#pragma once

#include "d3d9_device.h"
#include "d3d9_device_unknown.h"
#include "d3d9_surface.h"

namespace dxup {

  using Direct3DSwapChain9ExBase = D3D9DeviceUnknown<IDirect3DSwapChain9Ex>;
  class Direct3DSwapChain9Ex final : public Direct3DSwapChain9ExBase {

  public:

	Direct3DSwapChain9Ex(Direct3DDevice9Ex* device, D3DPRESENT_PARAMETERS* presentationParameters, IDXGISwapChain1* swapchain);

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;

    HRESULT STDMETHODCALLTYPE Present(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags) override;
    HRESULT STDMETHODCALLTYPE GetFrontBufferData(IDirect3DSurface9* pDestSurface) override;
    HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) override;
    HRESULT STDMETHODCALLTYPE GetRasterStatus(D3DRASTER_STATUS* pRasterStatus) override;
    HRESULT STDMETHODCALLTYPE GetDisplayMode(D3DDISPLAYMODE* pMode) override;
    HRESULT STDMETHODCALLTYPE GetPresentParameters(D3DPRESENT_PARAMETERS* pPresentationParameters) override;
    HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT* pLastPresentCount) override;
    HRESULT STDMETHODCALLTYPE GetPresentStats(D3DPRESENTSTATS* pPresentationStatistics) override;
    HRESULT STDMETHODCALLTYPE GetDisplayModeEx(D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) override;

    HRESULT PresentD3D11(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags, UINT d3d11Flags, bool ex);

    HRESULT Reset(D3DPRESENT_PARAMETERS* parameters);

    HRESULT TestSwapchain(HWND hDestWindowOverride, bool ex);
    HRESULT WaitForVBlank();

    static HRESULT Create(Direct3DDevice9Ex* device, D3DPRESENT_PARAMETERS* pPresentationParameters, Direct3DSwapChain9Ex** ppSwapChain);
  private:

    void clearResources();
    void rtBlit();

    Com<IDXGISwapChain1> m_swapchain;
    Com<IDXGIOutput1> m_output;
    D3DPRESENT_PARAMETERS m_presentationParameters;
    std::array<ComPrivate<Direct3DSurface9>, D3DPRESENT_BACK_BUFFERS_MAX_EX> m_buffers;
    std::array<ComPrivate<Direct3DSurface9>, D3DPRESENT_BACK_BUFFERS_MAX_EX> m_exposedBuffers; // these can be rts or actual things.
    bool m_rtRequired;
  };

}