#include "d3d9_swapchain.h"
#include "d3d9_surface.h"
#include <algorithm>

namespace dxup {

  Direct3DSwapChain9Ex::Direct3DSwapChain9Ex(Direct3DDevice9Ex* device, D3DPRESENT_PARAMETERS* presentationParameters, IDXGISwapChain1* swapchain)
    : Direct3DSwapChain9ExBase(device)
    , m_swapchain(swapchain) {
    Reset(presentationParameters);
  }

  HRESULT Direct3DSwapChain9Ex::Reset(D3DPRESENT_PARAMETERS* parameters) {
    CriticalSection cs(m_device);

    // Get info and crap!
    UINT bufferCount = std::min(1u, parameters->BackBufferCount);

    // Free crap!
    m_output = nullptr;

    for (size_t i = 0; i < bufferCount; i++) {
      if (m_buffers[i] != nullptr)
        m_buffers[i]->ClearResource();
    }

    // Set crap!

    m_presentationParameters = *parameters;

    HRESULT result = m_swapchain->ResizeBuffers(
      bufferCount,
      parameters->BackBufferWidth,
      parameters->BackBufferHeight,
      convert::makeUntypeless(convert::format(parameters->BackBufferFormat), false),
      0);

    if (FAILED(result)) {
      log::fail("ResizeBuffers failed in swapchain reset.");
      return D3DERR_INVALIDCALL;
    }

    result = m_swapchain->SetFullscreenState(!parameters->Windowed, nullptr);

    if (FAILED(result))
      log::warn("Failed to change fullscreen state.");

    // Make crap!

    for (UINT i = 0; i < bufferCount; i++) {
      Com<ID3D11Texture2D> texture;

      HRESULT result = m_swapchain->GetBuffer(i, __uuidof(ID3D11Texture2D), (void**)&texture);
      if (FAILED(result)) {
        log::warn("Failed to get swapchain buffer as ID3D11Texture2D.");
        continue;
      }

      DXUPResource* resource = DXUPResource::Create(m_device, texture.ptr(), D3DUSAGE_RENDERTARGET, D3DFMT_UNKNOWN);
      if (resource == nullptr) {
        log::warn("Failed to create DXUPResource for backbuffer.");
        continue;
      }

      D3D9ResourceDesc d3d9Desc;
      d3d9Desc.Discard = false;
      d3d9Desc.Format = parameters->BackBufferFormat;
      d3d9Desc.Usage = D3DUSAGE_RENDERTARGET;

      if (m_buffers[i] != nullptr)
        m_buffers[i]->SetResource(resource);
      else
        m_buffers[i] = new Direct3DSurface9(false, 0, 0, m_device, this, resource, d3d9Desc);
    }

    Com<IDXGIOutput> output;
    result = m_swapchain->GetContainingOutput(&output);
    if (FAILED(result)) {
      log::warn("Failed to get Swapchain IDXGIOutput");
      return D3DERR_INVALIDCALL;
    }

    result = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&m_output);

    if (FAILED(result)) {
      log::warn("Failed to get Swapchain IDXGIOutput1");
      return D3DERR_INVALIDCALL;
    }

    return D3D_OK;
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
    CriticalSection cs(m_device);

    return PresentD3D11(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags, 0, false);
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetFrontBufferData(IDirect3DSurface9* pDestSurface) {
    CriticalSection cs(m_device);
    log::stub("Direct3DSwapChain9Ex::GetFrontBufferData");

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) {
    CriticalSection cs(m_device);

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
    CriticalSection cs(m_device);

    log::stub("Direct3DSwapChain9Ex::GetRasterStatus");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetDisplayMode(D3DDISPLAYMODE* pMode) {
    CriticalSection cs(m_device);

    if (pMode == nullptr)
      return D3DERR_INVALIDCALL;

    pMode->Width = m_presentationParameters.BackBufferWidth;
    pMode->Height = m_presentationParameters.BackBufferHeight;
    pMode->Format = m_presentationParameters.BackBufferFormat;
    pMode->RefreshRate = m_presentationParameters.FullScreen_RefreshRateInHz;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetPresentParameters(D3DPRESENT_PARAMETERS* pPresentationParameters) {
    CriticalSection cs(m_device);

    if (!pPresentationParameters)
      return D3DERR_INVALIDCALL;

    *pPresentationParameters = m_presentationParameters;

    return D3D_OK;
  }

  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetLastPresentCount(UINT* pLastPresentCount) {
    CriticalSection cs(m_device);

    log::stub("Direct3DSwapChain9Ex::GetLastPresentCount");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetPresentStats(D3DPRESENTSTATS* pPresentationStatistics) {
    CriticalSection cs(m_device);

    log::stub("Direct3DSwapChain9Ex::GetPresentStats");
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetDisplayModeEx(D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
    CriticalSection cs(m_device);

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

  HRESULT Direct3DSwapChain9Ex::PresentD3D11(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags, UINT d3d11Flags, bool ex) {
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