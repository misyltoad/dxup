#include "d3d9_swapchain.h"
#include "d3d9_surface.h"

namespace dxapex {

  Direct3DSwapChain9Ex::Direct3DSwapChain9Ex(Direct3DDevice9Ex* device, D3DPRESENT_PARAMETERS* presentationParameters, IDXGISwapChain* swapchain)
    : Direct3DSwapChain9ExBase(device)
    , m_swapchain(swapchain)
    , m_presentationParameters(*presentationParameters) {

    DXGI_SWAP_CHAIN_DESC desc;
    m_swapchain->GetDesc(&desc);

    UINT buffers = desc.BufferCount;

    if (buffers > D3DPRESENT_BACK_BUFFERS_MAX_EX)
      buffers = D3DPRESENT_BACK_BUFFERS_MAX_EX;

    if (desc.SwapEffect == DXGI_SWAP_EFFECT_DISCARD)
      buffers = 1;

    for (size_t i = 0; i < buffers; i++)
    {
      Com<ID3D11Texture2D> texture;

      HRESULT result = m_swapchain->GetBuffer(i, __uuidof(ID3D11Texture2D), (void**)&texture);
      if (FAILED(result))
        return;
      
      m_buffers[i] = ref(new Direct3DSurface9(false, 0, m_device, this, texture.ptr(), D3DPOOL_DEFAULT, D3DUSAGE_RENDERTARGET));
    }

    m_swapchain->GetContainingOutput(&m_output);
  }

  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::QueryInterface(REFIID riid, void** ppvObj) {
    InitReturnPtr(ppvObj);

    if (!ppvObj)
      return E_POINTER;

    if (riid == __uuidof(IDirect3DSwapChain9Ex) || riid == __uuidof(IDirect3DSwapChain9) || riid == __uuidof(IUnknown)) {
      *ppvObj = ref(this);
      return D3D_OK;
    }

    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::Present(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags) {
    return PresentD3D11(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags, 0, false);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetFrontBufferData(IDirect3DSurface9* pDestSurface) {
    log::stub("Direct3DSwapChain9Ex::GetFrontBufferData");

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
    InitReturnPtr(ppBackBuffer);

    if (Type != D3DBACKBUFFER_TYPE_MONO)
    {
      log::warn("Attempted to use a stereo backbuffer. This is not supported!");
      return D3DERR_INVALIDCALL;
    }

    if (!ppBackBuffer || iBackBuffer > D3DPRESENT_BACK_BUFFERS_MAX_EX)
      return D3DERR_INVALIDCALL;

    if (m_buffers[iBackBuffer] == nullptr)
      return D3DERR_INVALIDCALL;

    *ppBackBuffer = ref(m_buffers[iBackBuffer]);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetRasterStatus(D3DRASTER_STATUS* pRasterStatus) {
    log::stub("Direct3DSwapChain9Ex::GetRasterStatus");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetDisplayMode(D3DDISPLAYMODE* pMode) {
    log::stub("Direct3DSwapChain9Ex::GetDisplayMode");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetPresentParameters(D3DPRESENT_PARAMETERS* pPresentationParameters) {
    if (!pPresentationParameters)
      return D3DERR_INVALIDCALL;

    *pPresentationParameters = m_presentationParameters;

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetLastPresentCount(UINT* pLastPresentCount)
  {
    log::stub("Direct3DSwapChain9Ex::GetLastPresentCount");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetPresentStats(D3DPRESENTSTATS* pPresentationStatistics)
  {
    log::stub("Direct3DSwapChain9Ex::GetPresentStats");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetDisplayModeEx(D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation)
  {
    log::stub("Direct3DSwapChain9Ex::GetDisplayModeEx");
    return D3D_OK;
  }

  HRESULT Direct3DSwapChain9Ex::WaitForVBlank() {
    m_output->WaitForVBlank();
    return D3D_OK;
  }

  HRESULT Direct3DSwapChain9Ex::TestSwapchain(HWND hDestWindowOverride, bool ex) {
    return PresentD3D11(nullptr, nullptr, hDestWindowOverride, nullptr, 0, DXGI_PRESENT_TEST, ex);
  }

  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::PresentD3D11(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags, UINT d3d11Flags, bool ex) {
    HRESULT result;

    if (hDestWindowOverride != nullptr) {
      log::warn("Present given with a window override. Not supported yet! Not presenting.");
      return D3DERR_INVALIDCALL;
    }

    if (d3d11Flags != 0) {
      result = m_swapchain->Present(0, d3d11Flags);
    }
    else {
      // We may need to do more here for rects...
      //m_swapchain->ResizeTarget
      //m_swapchain->ResizeBuffers
      result = m_swapchain->Present(0, d3d11Flags);
    }

    if (d3d11Flags & DXGI_PRESENT_TEST && FAILED(result))
        return D3DERR_DEVICELOST;
    else {

      if (result == DXGI_ERROR_DEVICE_REMOVED)
        return D3DERR_DEVICEREMOVED;

      if (ex) {

        if (result == DXGI_ERROR_DEVICE_HUNG)
          return D3DERR_DEVICEHUNG;

        if (result == DXGI_ERROR_DEVICE_RESET)
          return D3DERR_DEVICELOST;

      }
      else {

        if (result == DXGI_ERROR_DEVICE_RESET)
          return D3DERR_DEVICENOTRESET;

      }

      if (FAILED(result))
        return D3DERR_DRIVERINTERNALERROR;

    }

    return D3D_OK;
  }

}