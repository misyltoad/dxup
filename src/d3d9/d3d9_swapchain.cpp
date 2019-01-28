#include "d3d9_swapchain.h"
#include "d3d9_surface.h"
#include "d3d9_renderer.h"
#include "d3d9_interface.h"
#include <algorithm>

namespace dxup {

  Direct3DSwapChain9Ex::Direct3DSwapChain9Ex(Direct3DDevice9Ex* device, D3DPRESENT_PARAMETERS* presentationParameters, IDXGISwapChain1* swapchain)
    : Direct3DSwapChain9ExBase{ device }
    , m_swapchain{ swapchain }
    , m_rtRequired{ false } {
    this->Reset(presentationParameters);
  }

  HRESULT Direct3DSwapChain9Ex::Reset(D3DPRESENT_PARAMETERS* parameters) {
    CriticalSection cs(m_device);

    // Get info and crap!
    UINT bufferCount = std::max(1u, parameters->BackBufferCount);

    // Free crap!
    this->clearResources();

    // Set crap!

    m_presentationParameters = *parameters;

    DXGI_FORMAT format = convert::format(parameters->BackBufferFormat);
    format = convert::makeUntypeless(format, false);

    if (format == DXGI_FORMAT_B8G8R8X8_UNORM)
      format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is a simple fixup we can do to avoid a blit on both D3D11 native and older DXVK.

    HRESULT result = m_swapchain->ResizeBuffers(
      bufferCount,
      parameters->BackBufferWidth,
      parameters->BackBufferHeight,
      format,
      0);

    m_rtRequired = false;

    // dxvk opt for arbitrary swapchain.
    if (FAILED(result)) {
      DXGI_FORMAT forcedFormat = convert::makeSwapchainCompliant(format);
      log::msg("Reset: using rendertargets as intemediary for swapchain.");
      m_rtRequired = true;

      result = m_swapchain->ResizeBuffers(
        bufferCount,
        parameters->BackBufferWidth,
        parameters->BackBufferHeight,
        forcedFormat,
        0);
    }

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "Reset: D3D11 ResizeBuffers failed in swapchain reset.");

    if (!config::getBool(config::ForceWindowed)) {
      result = m_swapchain->SetFullscreenState(!parameters->Windowed, nullptr);

      if (FAILED(result))
        log::warn("Reset: failed to change fullscreen state!");
    }

    // Make crap!

    for (UINT i = 0; i < bufferCount; i++) {
      Com<ID3D11Texture2D> bufferTexture;

      HRESULT result = m_swapchain->GetBuffer(i, __uuidof(ID3D11Texture2D), (void**)&bufferTexture);
      if (FAILED(result)) {
        log::warn("reset: failed to get swapchain buffer as ID3D11Texture2D.");
        continue;
      }

      DXUPResource* resource = DXUPResource::Create(m_device, bufferTexture.ptr(), D3DUSAGE_RENDERTARGET, D3DFMT_UNKNOWN);
      if (resource == nullptr) {
        log::warn("reset: failed to create DXUPResource for backbuffer.");
        continue;
      }

      D3D9ResourceDesc d3d9Desc;
      d3d9Desc.Discard = false;
      d3d9Desc.Format = parameters->BackBufferFormat;
      d3d9Desc.Usage = D3DUSAGE_RENDERTARGET;

      if (m_buffers[i] != nullptr)
        m_buffers[i]->SetResource(resource);
      else
        m_buffers[i] = Direct3DSurface9::Wrap(0, 0, m_device, this, resource, d3d9Desc);

      if (m_rtRequired) {
        D3D11_TEXTURE2D_DESC rtDesc;
        rtDesc.Width = parameters->BackBufferWidth;
        rtDesc.Height = parameters->BackBufferHeight;
        rtDesc.MipLevels = 1;
        rtDesc.ArraySize = 1;
        rtDesc.Format = convert::makeTypeless(format);
        rtDesc.SampleDesc.Count = 1;
        rtDesc.SampleDesc.Quality = 0;
        rtDesc.Usage = D3D11_USAGE_DEFAULT;
        rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        rtDesc.CPUAccessFlags = 0;
        rtDesc.MiscFlags = 0;

        Com<ID3D11Texture2D> rtTexture;
        this->GetD3D11Device()->CreateTexture2D(&rtDesc, nullptr, &rtTexture);

        resource = DXUPResource::Create(m_device, rtTexture.ptr(), D3DUSAGE_RENDERTARGET, D3DFMT_UNKNOWN);
        if (resource == nullptr) {
          log::warn("reset: failed to create DXUPResource for rt passthrough.");
          continue;
        }
      }

      if (m_exposedBuffers[i] != nullptr)
        m_exposedBuffers[i]->SetResource(resource);
      else
        m_exposedBuffers[i] = Direct3DSurface9::Wrap(0, 0, m_device, this, resource, d3d9Desc);
    }

    Com<IDXGIOutput> output;
    result = m_swapchain->GetContainingOutput(&output);
    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "Reset: failed to get IDXGIOutput for swapchain.");

    result = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&m_output);

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "Reset: failed to upgrade IDXGIOutput to IDXGIOutput1 for swapchain.");

    return D3D_OK;
  }

  void Direct3DSwapChain9Ex::clearResources() {
    m_output = nullptr;

    for (size_t i = 0; i < m_buffers.size(); i++) {
      if (m_buffers[i] != nullptr)
        m_buffers[i]->ClearResource();
    }

    for (size_t i = 0; i < m_exposedBuffers.size(); i++) {
      if (m_exposedBuffers[i] != nullptr)
        m_exposedBuffers[i]->ClearResource();
    }
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

    return this->PresentD3D11(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags, 0, false);
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
      return log::d3derr(D3DERR_INVALIDCALL, "GetBackBuffer: stereo backbuffer requested.");

    if (!ppBackBuffer || iBackBuffer > D3DPRESENT_BACK_BUFFERS_MAX_EX)
      return log::d3derr(D3DERR_INVALIDCALL, "GetBackBuffer: backbuffer out of bounds.");

    if (m_exposedBuffers[iBackBuffer] == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetBackBuffer: invalid backbuffer requested (%d).", iBackBuffer);

    *ppBackBuffer = ref(m_exposedBuffers[iBackBuffer]);

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetRasterStatus(D3DRASTER_STATUS* pRasterStatus) {
    CriticalSection cs(m_device);

    if (pRasterStatus == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetRasterStatus: pRasterStatus was nullptr.");

    // There exists D3DKMTGetScanLine which could implement this.
    // However the header for it is DDI and it's not supported under Wine.
    // Just stubbing this for now, but returning something thats should at least make the games happy.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/content/d3dkmthk/nf-d3dkmthk-d3dkmtgetscanline
    static bool hasWarned = false;
    if (!hasWarned) {
      log::warn("GetRasterStatus: returning vblank.");
      hasWarned = true;
    }

    pRasterStatus->InVBlank = true;
    pRasterStatus->ScanLine = 0;
    
    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetDisplayMode(D3DDISPLAYMODE* pMode) {
    CriticalSection cs(m_device);

    if (pMode == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetDisplayMode: pMode was nullptr.");

    pMode->Width = m_presentationParameters.BackBufferWidth;
    pMode->Height = m_presentationParameters.BackBufferHeight;
    pMode->Format = m_presentationParameters.BackBufferFormat;
    pMode->RefreshRate = m_presentationParameters.FullScreen_RefreshRateInHz;

    return D3D_OK;
  }
  HRESULT STDMETHODCALLTYPE Direct3DSwapChain9Ex::GetPresentParameters(D3DPRESENT_PARAMETERS* pPresentationParameters) {
    CriticalSection cs(m_device);

    if (pPresentationParameters == nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "GetPresentParameters: pPresentationParameters was nullptr.");

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
    HRESULT result = m_output->WaitForVBlank();

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "WaitForVBlank: IDXGIOutput1::WaitForVBlank failed.");

    return D3D_OK;
  }

  HRESULT Direct3DSwapChain9Ex::TestSwapchain(HWND hDestWindowOverride, bool ex) {
    return this->PresentD3D11(nullptr, nullptr, hDestWindowOverride, nullptr, 0, DXGI_PRESENT_TEST, ex);
  }

  void Direct3DSwapChain9Ex::rtBlit() {
    // TODO! Do we need to change what buffer we do this with?
    this->GetD3D9Device()->GetRenderer()->blit(m_buffers[0].ptr(), m_exposedBuffers[0].ptr());
  }

  HRESULT Direct3DSwapChain9Ex::PresentD3D11(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags, UINT d3d11Flags, bool ex) {
    HRESULT result;

    if (hDestWindowOverride != nullptr)
      return log::d3derr(D3DERR_INVALIDCALL, "PresentD3D11: called with window override. Not presenting.");

    if (m_rtRequired && !(d3d11Flags & DXGI_PRESENT_TEST))
      this->rtBlit();

    UINT syncInterval = 0;

    if (config::getBool(config::RespectVSync)) {
      if (m_presentationParameters.PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE)
        syncInterval = 0;
      else if (m_presentationParameters.PresentationInterval == D3DPRESENT_INTERVAL_DEFAULT || m_presentationParameters.PresentationInterval == D3DPRESENT_INTERVAL_ONE)
        syncInterval = 1;
      else if (m_presentationParameters.PresentationInterval == D3DPRESENT_INTERVAL_TWO)
        syncInterval = 2;
      else if (m_presentationParameters.PresentationInterval == D3DPRESENT_INTERVAL_THREE)
        syncInterval = 3;
      else //if (m_presentationParameters.PresentationInterval == D3DPRESENT_INTERVAL_FOUR)
        syncInterval = 4;

      if (dwFlags & D3DPRESENT_FORCEIMMEDIATE)
        syncInterval = 0;
    }

    if (dwFlags & D3DPRESENT_DONOTWAIT)
      d3d11Flags |= DXGI_PRESENT_DO_NOT_WAIT;

    if (d3d11Flags & DXGI_PRESENT_TEST) {
      result = m_swapchain->Present(syncInterval, d3d11Flags);
    }
    else {
      // We may need to do more here for rects...
      //m_swapchain->ResizeTarget
      //m_swapchain->ResizeBuffers
      result = m_swapchain->Present(syncInterval, d3d11Flags);
    }

    if (d3d11Flags & DXGI_PRESENT_TEST && FAILED(result))
        return D3DERR_DEVICELOST;
    else {

      if (result == DXGI_ERROR_WAS_STILL_DRAWING)
        return D3DERR_WASSTILLDRAWING;

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

  HRESULT Direct3DSwapChain9Ex::Create(Direct3DDevice9Ex* device, D3DPRESENT_PARAMETERS* presentationParameters, Direct3DSwapChain9Ex** ppSwapChain) {
    InitReturnPtr(ppSwapChain);

    if (!ppSwapChain)
      return log::d3derr(D3DERR_INVALIDCALL, "CreateAdditionalSwapChain: ppSwapChain was nullptr.");

    DXGI_SWAP_CHAIN_DESC desc;
    memset(&desc, 0, sizeof(desc));

    UINT backBufferCount = std::max(1u, presentationParameters->BackBufferCount);

    DXGI_FORMAT format = convert::format(presentationParameters->BackBufferFormat);
    format = convert::makeUntypeless(format, false);

    if (format == DXGI_FORMAT_B8G8R8X8_UNORM)
      format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is a simple fixup we can do to avoid a blit on both D3D11 native and older DXVK.

    desc.BufferCount = backBufferCount;
    desc.BufferDesc.Width = presentationParameters->BackBufferWidth;
    desc.BufferDesc.Height = presentationParameters->BackBufferHeight;
    desc.BufferDesc.Format = format;
    desc.BufferDesc.RefreshRate.Numerator = 0;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    desc.OutputWindow = device->getWindow();
    desc.Windowed = config::getBool(config::ForceWindowed) ? true : presentationParameters->Windowed;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    //desc.SampleDesc.Count = (UINT)pPresentationParameters->MultiSampleType;

    //if (desc.SampleDesc.Count == 0)
    //  desc.SampleDesc.Count = 1;

    //desc.SampleDesc.Quality = pPresentationParameters->MultiSampleQuality;

    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    Com<Direct3D9Ex> parent;
    device->GetParent(&parent);

    Com<IDXGISwapChain> dxgiSwapChain;
    HRESULT result = parent->GetDXGIFactory()->CreateSwapChain(device->GetD3D11Device(), &desc, &dxgiSwapChain);

    // dxvk opt. for arbitrary swapchain
    if (FAILED(result)) {
      format = convert::makeSwapchainCompliant(format);
      desc.BufferDesc.Format = format;
      result = parent->GetDXGIFactory()->CreateSwapChain(device->GetD3D11Device(), &desc, &dxgiSwapChain);
    }

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "Swapchain - Create: failed to make D3D11 swapchain.");

    Com<IDXGISwapChain1> upgradedSwapchain;
    result = dxgiSwapChain->QueryInterface(__uuidof(IDXGISwapChain1), (void**)&upgradedSwapchain);

    if (FAILED(result))
      return log::d3derr(D3DERR_INVALIDCALL, "Swapchain - Create: failed to upgrade swapchain to IDXGISwapChain1!");

    parent->GetDXGIFactory()->MakeWindowAssociation(device->getWindow(), DXGI_MWA_NO_ALT_ENTER);

    *ppSwapChain = ref(new Direct3DSwapChain9Ex(device, presentationParameters, upgradedSwapchain.ptr()));
    return D3D_OK;
  }
}