#include "d3d9_interface.h"
#include "d3d9_device.h"
#include "../util/config.h"
#include <vector>

namespace dxapex {

  HRESULT STDMETHODCALLTYPE Direct3D9Ex::QueryInterface(REFIID riid, LPVOID* ppv) {
    InitReturnPtr(ppv);

    if (!ppv)
      return E_POINTER;

    if (riid == __uuidof(IDirect3D9Ex) || riid == __uuidof(IDirect3D9) || riid == __uuidof(IUnknown)) {
      *ppv = ref(this);
      return D3D_OK;
    }

    return E_NOINTERFACE;
  }

  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::RegisterSoftwareDevice(void* pInitializeFunction) {
    log::stub("Direct3D9Ex::RegisterSoftwareDevice");
    return D3D_OK;
  }
  UINT     STDMETHODCALLTYPE Direct3D9Ex::GetAdapterCount() {
    UINT AdapterCount = 0;
    Com<IDXGIAdapter> adapter;
    while (!FAILED(m_dxgiFactory->EnumAdapters(AdapterCount, &adapter)))
      AdapterCount++;

    return AdapterCount;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) {
    log::stub("Direct3D9Ex::GetAdapterIdentifier");
    return D3D_OK;
  }
  UINT     STDMETHODCALLTYPE Direct3D9Ex::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) {
    log::stub("Direct3D9Ex::GetAdapterModeCount");
    return 0;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) {
    log::stub("Direct3D9Ex::GetAdapterDisplayMode");
    return EnumAdapterModes(Adapter, D3DFMT_A8B8G8R8, 0, pMode);
  }

  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) {
    return D3D_OK;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) {
    return D3D_OK;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) {
    return D3D_OK;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) {
    return D3D_OK;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) {
    return D3D_OK;
  }

  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
    


    return D3D_OK;
  }

  HMONITOR STDMETHODCALLTYPE Direct3D9Ex::GetAdapterMonitor(UINT Adapter) {
    Com<IDXGIAdapter> adapter = nullptr;
    HRESULT Result = m_dxgiFactory->EnumAdapters(Adapter, &adapter);
    if (FAILED(Result))
      return NULL;

    Com<IDXGIOutput> output = nullptr;
    Result = adapter->EnumOutputs(0, &output);
    if (FAILED(Result))
      return NULL;

    DXGI_OUTPUT_DESC outputDesc;
    Result = output->GetDesc(&outputDesc);
    if (FAILED(Result))
      return NULL;

    return outputDesc.Monitor;
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
    InitReturnPtr(ppReturnedDeviceInterface);

    if (!pPresentationParameters)
      return D3DERR_INVALIDCALL;

    D3DDISPLAYMODEEX displayMode;
    displayMode.Size = sizeof(D3DDISPLAYMODEEX);
    displayMode.Width = pPresentationParameters->BackBufferWidth;
    displayMode.Height = pPresentationParameters->BackBufferHeight;
    displayMode.RefreshRate = 0;
    displayMode.Format = pPresentationParameters->BackBufferFormat;
    displayMode.ScanLineOrdering = D3DSCANLINEORDERING_PROGRESSIVE;

    return CreateDeviceEx(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &displayMode, (IDirect3DDevice9Ex**)ppReturnedDeviceInterface);
  }
  HRESULT   STDMETHODCALLTYPE Direct3D9Ex::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) {
    if (!pMode)
      return D3DERR_INVALIDCALL;

    D3DDISPLAYMODEEX exMode;
    HRESULT result = EnumAdapterModeFormatEx(Adapter, Format, NULL, Mode, &exMode);

    pMode->Format = exMode.Format;
    pMode->Height = exMode.Height;
    pMode->RefreshRate = exMode.RefreshRate;
    pMode->Width = exMode.Width;

    return result;
  }

  // Direct3D9Ex
  UINT     STDMETHODCALLTYPE Direct3D9Ex::GetAdapterModeCountEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter) {
    log::stub("Direct3D9Ex::GetAdapterModeCountEx");
    return D3D_OK;
  }
  HRESULT  STDMETHODCALLTYPE Direct3D9Ex::EnumAdapterModesEx(UINT Adapter, CONST D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) {
    return EnumAdapterModeFormatEx(Adapter, D3DFMT_A8B8G8R8, pFilter, Mode, pMode);
  }
  HRESULT  STDMETHODCALLTYPE Direct3D9Ex::GetAdapterDisplayModeEx(UINT Adapter, D3DDISPLAYMODEEX* pMode, D3DDISPLAYROTATION* pRotation) {
    log::stub("Direct3D9Ex::GetAdapterDisplayModeEx");
    return D3D_OK;
  }
  HRESULT  STDMETHODCALLTYPE Direct3D9Ex::CreateDeviceEx(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, D3DDISPLAYMODEEX* pFullscreenDisplayMode, IDirect3DDevice9Ex** ppReturnedDeviceInterface) {
    if (!ppReturnedDeviceInterface || !pPresentationParameters)
      return D3DERR_INVALIDCALL;

    InitReturnPtr(ppReturnedDeviceInterface);

    Com<IDXGIAdapter> adapter = nullptr;
    HRESULT Result = m_dxgiFactory->EnumAdapters(Adapter, &adapter);

    if (FAILED(Result))
      return D3DERR_DEVICELOST;

    UINT Flags = D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT | D3D11_CREATE_DEVICE_BGRA_SUPPORT; // Why isn't this a default?! ~ Josh

    if (config::getBool(config::Debug))
      Flags |= D3D11_CREATE_DEVICE_DEBUG;

    D3D_FEATURE_LEVEL FeatureLevels[] =
    {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
      D3D_FEATURE_LEVEL_9_3,
      D3D_FEATURE_LEVEL_9_2,
      D3D_FEATURE_LEVEL_9_1,
    };
    D3D_FEATURE_LEVEL Level = D3D_FEATURE_LEVEL_11_0;

    ID3D11Device* pDX11Device = nullptr;
    ID3D11DeviceContext* pImmediateContext = nullptr;

    Result = D3D11CreateDevice(
      adapter.ptr(),
      D3D_DRIVER_TYPE_UNKNOWN,
      nullptr,
      Flags,
      FeatureLevels,
      ARRAYSIZE(FeatureLevels),
      D3D11_SDK_VERSION,
      &pDX11Device,
      &Level,
      &pImmediateContext
    );

    if (FAILED(Result))
      return D3DERR_DEVICELOST;

    if (config::getBool(config::Debug)) {
      Com<ID3D11Debug> d3dDebug;
      if (SUCCEEDED(pDX11Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug))) {
        Com<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue))) {
          d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
          d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

          std::array<D3D11_MESSAGE_ID, 1> messagesToHide = {
          D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
          };

          D3D11_INFO_QUEUE_FILTER filter;
          memset(&filter, 0, sizeof(filter));
          filter.DenyList.NumIDs = messagesToHide.size();
          filter.DenyList.pIDList = &messagesToHide[0];
          d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
      }
    }

    D3DDEVICE_CREATION_PARAMETERS CreationParameters;
    CreationParameters.AdapterOrdinal = Adapter;
    CreationParameters.BehaviorFlags = BehaviorFlags;
    CreationParameters.DeviceType = D3DDEVTYPE_HAL;
    CreationParameters.hFocusWindow = hFocusWindow;

    RECT rect;
    GetWindowRect(hFocusWindow, &rect);

    if (!pPresentationParameters->BackBufferWidth)
      pPresentationParameters->BackBufferWidth = rect.right;

    if (!pPresentationParameters->BackBufferHeight)
      pPresentationParameters->BackBufferHeight = rect.bottom;

    if (!pPresentationParameters->BackBufferCount)
      pPresentationParameters->BackBufferCount = 1;

    if (pPresentationParameters->BackBufferFormat == D3DFMT_UNKNOWN)
      pPresentationParameters->BackBufferFormat = D3DFMT_A8B8G8R8;

    DeviceInitData initData;
    initData.adapter = adapter.ptr();
    initData.device = pDX11Device;
    initData.context = pImmediateContext;
    initData.ex = true;
    initData.parent = this;
    initData.creationParameters = &CreationParameters;
    initData.presentParameters = pPresentationParameters;
    initData.deviceType = DeviceType;

    *ppReturnedDeviceInterface = ref(new Direct3DDevice9Ex(&initData));

    return Result;
  }
  HRESULT  STDMETHODCALLTYPE Direct3D9Ex::GetAdapterLUID(UINT Adapter, LUID * pLUID) {
    if (!pLUID)
      return D3DERR_INVALIDCALL;

    Com<IDXGIAdapter> adapter = nullptr;
    if (!FAILED(m_dxgiFactory->EnumAdapters(Adapter, &adapter)))
    {
      DXGI_ADAPTER_DESC adapterDesc;      

      if (!FAILED(adapter->GetDesc(&adapterDesc)))
        *pLUID = adapterDesc.AdapterLuid;
      else
        return D3DERR_INVALIDCALL;
    }

    return D3D_OK;
  }

  HRESULT Direct3D9Ex::EnumAdapterModeFormatEx(UINT Adapter, D3DFORMAT Format, const D3DDISPLAYMODEFILTER* pFilter, UINT Mode, D3DDISPLAYMODEEX* pMode) {
    if (!pMode)
      return D3DERR_INVALIDCALL;

    Com<IDXGIAdapter> adapter = nullptr;
    HRESULT Result = m_dxgiFactory->EnumAdapters(Adapter, &adapter);
    if (FAILED(Result))
      return D3DERR_INVALIDCALL;

    Com<IDXGIOutput> output = nullptr;
    Result = adapter->EnumOutputs(0, &output);
    if (FAILED(Result))
      return D3DERR_INVALIDCALL;

    // This is disgusting, what the fuck MS?! ~ Josh
    UINT ModeCount = 0;
    DXGI_FORMAT dxgiFormat = convert::format(Format);
    Result = output->GetDisplayModeList(dxgiFormat, 0, &ModeCount, nullptr);

    if (FAILED(Result) || ModeCount <= Mode)
      return D3DERR_INVALIDCALL;

    const uint16_t StackMaxCount = 32;

    DXGI_MODE_DESC stackDescs[StackMaxCount];
    DXGI_MODE_DESC* pDescs = stackDescs;
    std::vector<DXGI_MODE_DESC> heapDescs;

    if (ModeCount > StackMaxCount) {
      heapDescs.resize(ModeCount);
      pDescs = &heapDescs[0];
    }

    std::reverse(pDescs, pDescs + StackMaxCount);

    Result = output->GetDisplayModeList(dxgiFormat, 0, &ModeCount, pDescs);
    if (FAILED(Result))
      return D3DERR_INVALIDCALL;

    DXGI_MODE_DESC& RequestedMode = pDescs[Mode];
    pMode->Format = convert::format(RequestedMode.Format);
    pMode->Width = RequestedMode.Width;
    pMode->Height = RequestedMode.Height;
    pMode->RefreshRate = RequestedMode.RefreshRate.Denominator;
    pMode->ScanLineOrdering = convert::scanlineOrdering(RequestedMode.ScanlineOrdering);
    pMode->Size = sizeof(D3DDISPLAYMODEEX);

    return D3D_OK;
  }

  // dxapex

  void Direct3D9Ex::GetDXGIFactory(IDXGIFactory** factory) {
    *factory = ref(m_dxgiFactory);
  }

}